/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2015-2016 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  $Id: jpegutil.c 2416 2016-03-05 12:23:35Z roi $
 */

/* 
 *  JPEG用ユーティリティ
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "integer.h"
#include "mdcard_play.h"
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */


static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};

static const char * progname;	/* program name for error messages */
static MEMDEV outmdev;
static FILE   outmfile;

void
exit(int __status)
{
	ID tid;

	get_tid(&tid);
	syslog_2(LOG_ERROR, "exit(%d) task(%d) !", __status, tid);
	slp_tsk();
}

void
abort(void)
{
	ID tid;

	get_tid(&tid);
	syslog_1(LOG_ERROR, "abort task(%d) !", tid);
	slp_tsk();
}


/*
 *  メモリファイル用１バイト取り込み
 */
static
int func_in(void* mfile)
{
	MEMDEV *mdev = (MEMDEV *)(((FILE*)mfile)->_dev);
	int val = -1;

	if(mdev->fsize > mdev->csize){
		val = mdev->head[mdev->csize++];
	}
	return val;
}

/*
 *  メモリファイル用データ取り込み
 */
static
int func_ins(void* mfile, unsigned int size, char *p)
{
	MEMDEV *mdev = (MEMDEV *)(((FILE*)mfile)->_dev);
	int val = 0;

	if(mdev->fsize >= (mdev->csize+size)){
		BCOPY(&mdev->head[mdev->csize], p, size);
		mdev->csize += size;
		val = size;
	}
	return val;
}

/*
 *  メモリファイル用１バイト書込み
 */
static
void func_out(void* mfile, int val)
{
	MEMDEV *mdev = (MEMDEV *)(((FILE*)mfile)->_dev);

	if(mdev->fsize > mdev->csize){
		mdev->head[mdev->csize++] = val;
	}
}

/*
 *  メモリファイル用データ書込み
 */
static
int func_outs(void* mfile, unsigned int size, char *p)
{
	MEMDEV *mdev = (MEMDEV *)(((FILE*)mfile)->_dev);
	int val = 0;

	if(mdev->fsize >= (mdev->csize+size)){
		BCOPY(p, &mdev->head[mdev->csize], size);
		mdev->csize += size;
		val = size;
	}
	return val;
}

/*
 *  メモリファイル用フラッシュ
 */
static
int func_flush(struct __msFILE *file)
{
	return 0;
}

/*
 *  メモリコピー
 */
void
bcopy2(uint8_t *s, uint8_t *d, int len)
{
	int i;
	for(i = 0 ; i < len ; i++)
		*d++ = *s++;
}

/*
 *  JPEGイメージ変換プログラム
 */
static
void jcopy(uint8_t *s, uint8_t *d, int len)
{
	int i;
	for(i = 0 ; i < len ; i++){
		*(d + 2) = *s;
		*d = *(s + 2);
		*(d + 1) = *(s + 1);
		d += 3;
		s += 3;
	}
}

/*
 *  メモリファイルハンドラを作成
 */
MEMDEV *
setup_memory_file(FILE *st, uint8_t *addr, uint32_t size)
{
	outmdev.head  = addr;
	outmdev.csize = 0;
	outmdev.fsize = size;

	st->_flags = 0;
	st->_dev = (void *)&outmdev;
	st->_func_in   = func_in;
	st->_func_ins  = func_ins;
	st->_func_out  = func_out;
	st->_func_outs = func_outs;
	st->_func_flush = func_flush;
	return &outmdev;
}

/*
 * Marker processor for COM and interesting APPn markers.
 * This replaces the library's built-in processor, which just skips the marker.
 * We want to print out the marker as text, to the extent possible.
 * Note this code relies on a non-suspending data source.
 */

LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
/* Read next byte */
{
	struct jpeg_source_mgr * datasrc = cinfo->src;

	if (datasrc->bytes_in_buffer == 0) {
		if (! (*datasrc->fill_input_buffer) (cinfo))
			ERREXIT(cinfo, JERR_CANT_SUSPEND);
	}
	datasrc->bytes_in_buffer--;
	return GETJOCTET(*datasrc->next_input_byte++);
}


METHODDEF(boolean)
print_text_marker (j_decompress_ptr cinfo)
{
	boolean traceit = (cinfo->err->trace_level >= 1);
	INT32 length;
	unsigned int ch;
	unsigned int lastch = 0;
	char *outstr;
	unsigned int i = 0;

	length = jpeg_getc(cinfo) << 8;
	length += jpeg_getc(cinfo);
	outstr = malloc(length);
	length -= 2;			/* discount the length word itself */

	if (traceit) {
		if (cinfo->unread_marker == JPEG_COM)
			syslog_1(LOG_ERROR, "Comment, length %ld:", (long) length);
		else			/* assume it is an APPn otherwise */
			syslog_2(LOG_ERROR, "APP%d, length %ld:",
				cinfo->unread_marker - JPEG_APP0, (long) length);
	}

	while (--length >= 0) {
		ch = jpeg_getc(cinfo);
		if (traceit) {
      /* Emit the character in a readable form.
       * Nonprintables are converted to \nnn form,
       * while \ is converted to \\.
       * Newlines in CR, CR/LF, or LF form will be printed as one newline.
       */
			if (ch == '\r') {
				outstr[i] = 0;
				syslog_1(LOG_NOTICE, "%s", outstr);
				i = 0;
			} else if (ch == '\n') {
				if (lastch != '\r'){
					outstr[i] = 0;
					syslog_1(LOG_NOTICE, "%s", outstr);
					i = 0;
				}
			} else if (ch == '\\') {
				outstr[i++] = '\\';
				outstr[i++] = '\\';
			} else if (ch >= ' ' && ch < 0x7f) {
				outstr[i++] = ch;
			} else {
				sprintf(&outstr[i], "\\%03o", ch);
				i += 4;
			}
			lastch = ch;
		}
	}

	if(traceit && i > 0){
		outstr[i] = 0;
		syslog_1(LOG_NOTICE, "%s", outstr);
	}
	dly_tsk(200);
	free(outstr);
	return TRUE;
}

/*
 *  JPEGからPPMへの変換
 */
int
jpeg2ppm(const char *infile, uint8_t *buffer, uint32_t size)
{
#ifndef NOTUSE_JPEG
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	djpeg_dest_ptr dest_mgr = NULL;
	FILE * input_file;
	MEMDEV *mdev;
	JDIMENSION num_scanlines;

	progname = "djpeg";		/* in case C library doesn't provide it */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	/* Add some application-specific error messages (from cderror.h) */
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;
	/* Insert custom marker processor for COM and APP12.
	 * APP12 is used by some digital camera makers for textual info,
	 * so we provide the ability to display it as text.
	 * If you like, additional APPn marker types can be selected for display,
	 * but don't try to override APP0 or APP14 this way (see libjpeg.doc).
	 */
	jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
	jpeg_set_marker_processor(&cinfo, JPEG_APP0+12, print_text_marker);
	if ((input_file = fopen(infile, READ_BINARY)) == NULL) {
		syslog_2(LOG_ERROR, "%s: can't open %s", progname, infile);
		return 0;
	}
	/* Specify data source for decompression */
	jpeg_stdio_src(&cinfo, input_file);
	/* Read file header, set default decompression parameters */
	(void) jpeg_read_header(&cinfo, TRUE);
	dest_mgr = jinit_write_ppm(&cinfo);
	mdev = setup_memory_file(&outmfile, (uint8_t *)buffer, size);
	dest_mgr->output_file = &outmfile;
	/* Start decompressor */
	(void) jpeg_start_decompress(&cinfo);

	/* Write output file header */
	(*dest_mgr->start_output) (&cinfo, dest_mgr);
	/* Process data */
	while (cinfo.output_scanline < cinfo.output_height) {
		num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
				dest_mgr->buffer_height);
		(*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
	}
	/* Finish decompression and release memory.
	 * I must do it in this order because output module has allocated memory
	 * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
	 */
	(*dest_mgr->finish_output) (&cinfo, dest_mgr);
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	/* Close files, if we opened them */
	fclose(input_file);
	return mdev->csize;
#else
	return 0;
#endif
}

/*
 *  PPMからRGBへの変換
 */
int
ppm2rgb(uint8_t *rgbaddr, int *pwidth, int *pheight)
{
	char *p6addr = (char *)rgbaddr + BMP_HEAD_SIZE - P6_HEAD_SIZE;
	uint8_t *laddr, *p1, *p2;
	int id, height, width, lcount, i;

	if(rgbaddr == NULL)
		return 0;
	sscanf(p6addr+1, "%d %d %d", &id, &width, &height);
	if(id != 6)
		return 0;
	bzero(rgbaddr, BMP_HEAD_SIZE-P6_HEAD_SIZE);
	rgbaddr[10] = BMP_HEAD_SIZE;
	rgbaddr[18] = width & 0xff;
	rgbaddr[19] = width >> 8;
	rgbaddr[22] = height & 0xff;
	rgbaddr[23] = height >> 8;
	rgbaddr[28] = 0x18;

	laddr = (uint8_t *)malloc(width*3);
	lcount = height / 2;
	for(i = 0 ; i < lcount ; i++){
		p1 = rgbaddr+BMP_HEAD_SIZE + (i * width * 3);
		p2 = rgbaddr+BMP_HEAD_SIZE + ((height-i-1) * width * 3);
		jcopy(p1, laddr, width);
		jcopy(p2, p1, width);
		BCOPY(laddr, p2, width*3);
	}
	if((height & 1) != 0){
		p1 = rgbaddr+BMP_HEAD_SIZE + (i * width * 3);
		jcopy(p1, laddr, width);
		BCOPY(laddr, p1, width*3);
	}
	free(laddr);
	*pwidth = width;
	*pheight= height;

	return width * height * 3 + BMP_HEAD_SIZE;
}

