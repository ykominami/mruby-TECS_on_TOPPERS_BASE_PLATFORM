/* #[<PREAMBLE>]#
 * Don't edit the comments between #[<...>]# and #[</...>]#
 * These comment are used by tecsmerege when merging.
 *
 * #[</PREAMBLE>]# */

/* Put prototype declaration and/or variale definition here #_PAC_# */
#include "tSerialPortWrapper_tecsgen.h"

#ifndef E_OK
#define	E_OK	0		/* success */
#define	E_ID	(-18)	/* illegal ID */
#endif

/* entry port function #_TEPF_# */
/* #[<ENTRY_PORT>]# eSerialPort
 * entry port: eSerialPort
 * signature:  sSerialPort
 * context:    task
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eSerialPort_open
 * name:         eSerialPort_open
 * global_name:  tSerialPortWrapper_eSerialPort_open
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSerialPort_open(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* Put statements here #_TEFB_# */
    ercd = serial_opn_por( ATTR_portid );

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSerialPort_close
 * name:         eSerialPort_close
 * global_name:  tSerialPortWrapper_eSerialPort_close
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSerialPort_close(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* Put statements here #_TEFB_# */
    ercd = serial_cls_por( ATTR_portid );

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSerialPort_read
 * name:         eSerialPort_read
 * global_name:  tSerialPortWrapper_eSerialPort_read
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER_UINT
eSerialPort_read(CELLIDX idx, char_t* buffer, uint_t length)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* Put statements here #_TEFB_# */
    ercd = serial_rea_dat( ATTR_portid, buffer, length );

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSerialPort_write
 * name:         eSerialPort_write
 * global_name:  tSerialPortWrapper_eSerialPort_write
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER_UINT
eSerialPort_write(CELLIDX idx, const char_t* buffer, uint_t length)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* Put statements here #_TEFB_# */
    ercd = serial_wri_dat( ATTR_portid, buffer, length );

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSerialPort_control
 * name:         eSerialPort_control
 * global_name:  tSerialPortWrapper_eSerialPort_control
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSerialPort_control(CELLIDX idx, uint_t ioControl)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* Put statements here #_TEFB_# */
    ercd = serial_ctl_por( ATTR_portid, ioControl );

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSerialPort_refer
 * name:         eSerialPort_refer
 * global_name:  tSerialPortWrapper_eSerialPort_refer
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSerialPort_refer(CELLIDX idx, T_SERIAL_RPOR *pk_rpor)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* Put statements here #_TEFB_# */
    ercd = serial_ref_por( ATTR_portid, pk_rpor );

	return(ercd);
}

/* #[<ENTRY_PORT>]# enSerialPort
 * entry port: enSerialPort
 * signature:  snSerialPort
 * context:    task
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# enSerialPort_getChar
 * name:         enSerialPort_getChar
 * global_name:  tSerialPortWrapper_enSerialPort_getChar
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
bool_t
enSerialPort_getChar(CELLIDX idx, char_t* p_char)
{
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		/* Write error processing code here */
	} /* end if VALID_IDX(idx) */
	(void)p_cellcb;

	/* Put statements here #_TEFB_# */
	syslog( LOG_ALERT, "enSerialPort_getChar no supportd");
	return false;
}

/* #[<POSTAMBLE>]#
 *   Put non-entry functions below.
 * #[</POSTAMBLE>]#*/
