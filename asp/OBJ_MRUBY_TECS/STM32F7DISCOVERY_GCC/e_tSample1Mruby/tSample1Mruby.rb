p "Entering mruby script."

=begin
*************
ブリッジを設置したセル
cell tSerialPort SerialPort1
cell tSysLog SysLog
cell tSerialPortWrapper SerialPort1
cell tSysLogWrapper SysLog
cell tLogTask LogTask
cell tKernel ASPKernel
cell tTask MainTask
cell tTask Task1
cell tTask Task2
cell tTask Task3
cell tCyclicHandler CyclicHandler
cell tAlarmHandler AlarmHandler
cell tSample1Mruby Sample1

*************
TECS ジェネレータ MrubyBridgePlugin (MrubyBridgeCellPlugin, MrubyBridgeSignaturePlugin) のメッセージ
TECC セル(受け口) へアクセスするために必要な情報が出力される．
MrubyBrdigePlugin で扱えない型を引数に持つ関数が、autoexclude により自動除外されている．

MrubyBridgeCellPlugin: [cell.port] SerialPort1.eSerialPort => [mruby instance] TECS::TsSerialPort.new( 'SerialPort1eSerialPortBridge' )
MrubyBridgeCellPlugin: [cell.port] SerialPort1.enSerialPort => [mruby instance] TECS::TsnSerialPort.new( 'SerialPort1enSerialPortBridge' )
MrubyBridgePlugin: [pointer]  char_t* => [class] TECS::CharPointer
MrubyBridgePlugin: [struct]   T_SERIAL_RPOR => [class] TECS::StructTAG_T_SERIAL_RPOR
MrubyBridgePlugin: [signature] ::sSerialPort => [celltype] nMruby::tsSerialPort => [class] TECS::TsSerialPort
MrubyBridgePlugin: join your VM's cInitialize to VM_TECSInitializer.eInitialize
MrubyBridgePlugin: [signature] ::snSerialPort => [celltype] nMruby::tsnSerialPort => [class] TECS::TsnSerialPort
MrubyBridgeCellPlugin: [cell.port] SysLog.eSysLog => [mruby instance] TECS::TsSysLog.new( 'SysLogeSysLogBridge' )
gen/tmp_MrubyBridgeCellPlugin_1.cdl:4:86: info: MRI9999 loginfo: type intptr_t[6] not allowed for struct member, write automatcally excluded
gen/tmp_MrubyBridgeCellPlugin_1.cdl:4:86: info: MRI9999 loginfo: type intptr_t[6] not allowed for struct member, read automatcally excluded
MrubyBridgePlugin: [struct]   T_SYSLOG_RLOG => [class] TECS::Structt_syslog_rlog
MrubyBridgePlugin: [signature] ::sSysLog => [celltype] nMruby::tsSysLog => [class] TECS::TsSysLog
MrubyBridgeCellPlugin: [cell.port] ASPKernel.eKernel => [mruby instance] TECS::TsKernel.new( 'ASPKerneleKernelBridge' )
MrubyBridgePlugin: [pointer]  ID* => [class] TECS::IntPointer
MrubyBridgePlugin: [pointer]  SYSTIM* => [class] TECS::ULongPointer
MrubyBridgePlugin: [signature] ::sKernel => [celltype] nMruby::tsKernel => [class] TECS::TsKernel
MrubyBridgeCellPlugin: [cell.port] MainTask.eTask => [mruby instance] TECS::TsTask.new( 'MainTaskeTaskBridge' )
MrubyBridgePlugin: [struct]   T_RTSK => [class] TECS::Structt_rtsk
MrubyBridgePlugin: [signature] ::sTask => [celltype] nMruby::tsTask => [class] TECS::TsTask
MrubyBridgeCellPlugin: [cell.port] Task1.eTask => [mruby instance] TECS::TsTask.new( 'Task1eTaskBridge' )
MrubyBridgeCellPlugin: [cell.port] Task2.eTask => [mruby instance] TECS::TsTask.new( 'Task2eTaskBridge' )
MrubyBridgeCellPlugin: [cell.port] Task3.eTask => [mruby instance] TECS::TsTask.new( 'Task3eTaskBridge' )
MrubyBridgeCellPlugin: [cell.port] CyclicHandler.eCyclic => [mruby instance] TECS::TsCyclic.new( 'CyclicHandlereCyclicBridge' )
MrubyBridgePlugin: [struct]   T_RCYC => [class] TECS::Structt_rcyc
MrubyBridgePlugin: [signature] ::sCyclic => [celltype] nMruby::tsCyclic => [class] TECS::TsCyclic
MrubyBridgeCellPlugin: [cell.port] AlarmHandler.eAlarm => [mruby instance] TECS::TsAlarm.new( 'AlarmHandlereAlarmBridge' )
MrubyBridgePlugin: [struct]   T_RALM => [class] TECS::Structt_ralm
MrubyBridgePlugin: [signature] ::sAlarm => [celltype] nMruby::tsAlarm => [class] TECS::TsAlarm
MrubyBridgeCellPlugin: [cell.port] Sample1.eMainTask => [mruby instance] TECS::TsTaskBody.new( 'Sample1eMainTaskBridge' )
MrubyBridgePlugin: [signature] ::sTaskBody => [celltype] nMruby::tsTaskBody => [class] TECS::TsTaskBody
*************

=end

#  シリアルインタフェースドライバの動作制御用のための定数 (serial.h)
IOCTL_NULL = 0               # /* 指定なし */
IOCTL_ECHO = 1               # /* 受信した文字をエコーバック */
IOCTL_CRLF = 0x10            # /* LFを送信する前にCRを付加 */
IOCTL_FCSND = 0x0100         # /* 送信に対してフロー制御を行う */
IOCTL_FCANY = 0x0200         # /* どのような文字でも送信再開 */
IOCTL_FCRCV = 0x0400         # /* 受信に対してフロー制御を行う */

# tSampleMruby.h
MAIN_PRIORITY	= 5		# /* メインタスクの優先度 */
# /* HIGH_PRIORITYより高くすること */

HIGH_PRIORITY	= 9		# /* 並列に実行されるタスクの優先度 */
MID_PRIORITY	= 10
LOW_PRIORITY	= 11

#  ログ情報の重要度の定義  (t_syslog.h)
LOG_EMERG		=	(0)		# /* シャットダウンに値するエラー */
LOG_ALERT		=	(1)
LOG_CRIT		=	(2)
LOG_ERROR		=	(3)		# /* システムエラー */
LOG_WARNING		=	(4)		# /* 警告メッセージ */
LOG_NOTICE		=	(5)
LOG_INFO		=	(6)
LOG_DEBUG		=	(7)		# /* デバッグ用メッセージ */

def LOG_UPTO prio
    ((1 << ((prio) + 1)) - 1)
end

def LOG_MASK(prio)
    (1 << (prio))
end

#----------------------------------------
tskno = 1     # 1..3

p "CreateBridge"
SerialPort = TECS::TsSerialPort.new( 'SerialPort1eSerialPortBridge' )
SerialPortN = TECS::TsnSerialPort.new( 'SerialPort1enSerialPortBridge' )
SysLog = TECS::TsSysLog.new( 'SysLogeSysLogBridge' )
ASPKernel = TECS::TsKernel.new( 'ASPKerneleKernelBridge' )
MainTask = TECS::TsTask.new( 'MainTaskeTaskBridge' )
CyclicHandler = TECS::TsCyclic.new( 'CyclicHandlereCyclicBridge' )
AlarmHandler =TECS::TsAlarm.new( 'AlarmHandlereAlarmBridge' )
SampleHandler = TECS::TsTaskBody.new( 'Sample1eMainTaskBridge' )

SampleTasks = []
SampleTasks[0] = TECS::TsTask.new( 'Task1eTaskBridge' )
SampleTasks[1]  = TECS::TsTask.new( 'Task2eTaskBridge' )
SampleTasks[2]  = TECS::TsTask.new( 'Task3eTaskBridge' )

# SVC_PERROR(cSysLog_mask(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
SysLog.mask( LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG) )
# syslog(LOG_NOTICE, "Sample program starts.");
p "Sample program starts."

# /*
#  *  シリアルポートの初期化
#  *
#  *  システムログタスクと同じシリアルポートを使う場合など，シリアル
#  *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
#  *  ない．
#  */

# ercd = cSerialPort_open();
ercd = SerialPort.open
# if (ercd < 0 && MERCD(ercd) != E_OBJ) {
#    syslog(LOG_ERROR, "%s (%d) reported by `cSerialPort_open'.",
#                                itron_strerror(ercd), SERCD(ercd));
#
#
# }
if ercd < 0 then
    p "error SerialPort.open"
end

p "control"
# SVC_PERROR(cSerialPort_control(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV));
SerialPort.control( IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV )
p "control end!"

=begin

/*
  *  ループ回数の設定
 *
 *  TASK_LOOPがマクロ定義されている場合，測定せずに，TASK_LOOPに定
 *  義された値を，タスク内でのループ回数とする．
 *
 *  MEASURE_TWICEがマクロ定義されている場合，1回目の測定結果を捨て
 *  て，2回目の測定結果を使う．1回目の測定は長めの時間が出るため．
 */
#ifdef TASK_LOOP
task_loop = TASK_LOOP;
#else /* TASK_LOOP */

#ifdef MEASURE_TWICE
task_loop = LOOP_REF;
SVC_PERROR(getTime(&stime1));
for (i = 0; i < task_loop; i++);
SVC_PERROR(getTime(&stime2));
#endif /* MEASURE_TWICE */

task_loop = LOOP_REF;
SVC_PERROR(getTime(&stime1));
for (i = 0; i < task_loop; i++);
SVC_PERROR(getTime(&stime2));
task_loop = LOOP_REF * 400UL / (stime2 - stime1);

#endif /* TASK_LOOP */
tex_loop = task_loop / 5;
=end

# /*
#   *  タスクの起動
#  */
# SVC_PERROR(cTask_activate( 1 ));
# SVC_PERROR(cTask_activate( 2 ));
# SVC_PERROR(cTask_activate( 3 ));
SampleTasks[0].activate
SampleTasks[1].activate
SampleTasks[2].activate

charpoi = TECS::CharPointer.new( 1 )


# /*
#  *  メインループ
#  */
# do {
#     SVC_PERROR(cSerialPort_read(&c, 1));
while true
    SerialPort.read(charpoi, 1)
    p "c=#{charpoi[0]}"
    c = charpoi[0]

    # switch (c) {
    case c

    # case 'e':
    # case 's':
    # case 'S':
    # case 'd':
    # case 'y':
    # case 'Y':
    # case 'z':
    # case 'Z':
    #     message[tskno-1] = c;
    #     break;
#         e    s   S   d    y    Y   z    Z
    when 101 ,115, 83, 109, 121, 89, 122, 90
    p "message to sub task #{c} to ignore"

    # case '1':
    #     tskno = 1;
    #     break;
    when 49  # 1
        tskno = 1
    # case '2':
    # tskno = 2;
    # break;
    when 50  # 2
        tskno = 2
    # case '3':
    #     tskno = 3;
    #     break;
    when 51  # 3
        tskno = 3
    # case 'a':
    #     syslog(LOG_INFO, "#cTask_activate(%d)", tskno);
    #     SVC_PERROR(cTask_activate(tskno));
    #     break;
    when 97   # a
        p "SampleTask[#{tskno}].activate"
        SampleTasks[tskno - 1].activate
    # case 'A':
    #     syslog(LOG_INFO, "#cTask_cancelActivate(%d)", tskno);
    #     SVC_PERROR(cTask_cancelActivate(tskno));

    #     if (ercd >= 0) {
    #         syslog(LOG_NOTICE, "cTask_cancelActivate(%d) returns %d", tskno, ercd);
    #     }
    #     break;
    when 65   # A
        p "SampleTask[#{tskno}].cancelActivate"
        SampleTasks[tskno - 1].cancelActivate

    # case 't':
    #     syslog(LOG_INFO, "#cTask_terminate(%d)", tskno);
    #     SVC_PERROR(cTask_terminate(tskno));
    #     break;
    when 116  # t
        p "SampleTask[#{tskno}].terminate"
        SampleTasks[tskno - 1].terminate
    # case '>':
    # syslog(LOG_INFO, "#cTask_changePriority(%d, HIGH_PRIORITY)", tskno);
    # SVC_PERROR(cTask_changePriority(tskno, HIGH_PRIORITY));
    # break;
    when 0x3e  # >
        p "SampleTask[#{tskno}].changePriority( HIGH_PRIORITY=#{HIGH_PRIORITY} )"
        SampleTasks[tskno - 1].changePriority( HIGH_PRIORITY )

    # case '=':
    #     syslog(LOG_INFO, "#cTask_changePriority(%d, MID_PRIORITY)", tskno);
    #     SVC_PERROR(cTask_changePriority(tskno, MID_PRIORITY));
    #     break;
    when 0x3d  # =
        p "SampleTask[#{tskno}].changePriority( MID_PRIORITY=#{MID_PRIORITY} )"
        SampleTasks[tskno - 1].changePriority( MID_PRIORITY )

    # case '<':
    #     syslog(LOG_INFO, "#(cTask_changePriority(%d, LOW_PRIORITY)", tskno);
    #     SVC_PERROR(cTask_changePriority(tskno, LOW_PRIORITY));
    #     break;
    when 0x3c  # <
        p "SampleTask[#{tskno}].changePriority( LOW_PRIORITY=#{LOW_PRIORITY} )"
        SampleTasks[tskno - 1].changePriority( LOW_PRIORITY )
    # case 'G':
    #     syslog(LOG_INFO, "#cTask_getPriority(%d, &tskpri)", tskno);
    #     SVC_PERROR(ercd = cTask_getPriority(tskno, &tskpri));
    #     if (ercd >= 0) {
    #         syslog(LOG_NOTICE, "priority of task %d is %d", tskno, tskpri);
    #     }
    #     break;
    when 71   # G
        pPri = TECS::IntPointer.new(1)
        p "SampleTask[#{tskno}].getPriority( pPri )"
        SampleTasks[tskno - 1].getPriority( pPri )
        p "priority of SampleTask[#{tskno}] is #{pPri[0]}"
    # case 'w':
    #     syslog(LOG_INFO, "#cTask_wakeup(%d)", tskno);
    #     SVC_PERROR(cTask_wakeup(tskno));
    #     break;
    when 119  # w
        p "SampleTask[#{tskno}].wakeup"
        SampleTasks[tskno - 1].wakeup
    # case 'W':
    #     syslog(LOG_INFO, "#cTask_cancelWakeup(%d)", tskno);
    #     SVC_PERROR(ercd = cTask_cancelWakeup(tskno));
    #     if (ercd >= 0) {
    #         syslog(LOG_NOTICE, "cTask_cancelWakeup(%d) returns %d", tskno, ercd);
    #     }
    #     break;
    when 87   # W
        p "SampleTask[#{tskno}].cancelWakeup"
        SampleTasks[tskno - 1].cancelWakeup
    # case 'l':
    #     syslog(LOG_INFO, "#cTask_releaseWait(%d)", tskno);
    #     SVC_PERROR(cTask_releaseWait(tskno));
    #     break;
    when 108  # l
        p "SampleTask[#{tskno}].releaseWait"
        SampleTasks[tskno - 1].releaseWait
    # case 'u':
    #     syslog(LOG_INFO, "#cTask_suspend(%d)", tskno);
    #     SVC_PERROR(cTask_suspend(tskno));
    #     break;
    when 117  # u
        p "SampleTask[#{tskno}].suspend"
        SampleTasks[tskno - 1].suspend
    # case 'm':
    #     syslog(LOG_INFO, "#cTask_resume(%d)", tskno);
    #     SVC_PERROR(cTask_resume(tskno));
    #     break;
    when 109  # m
        p "SampleTask[#{tskno}].resume"
        SampleTasks[tskno - 1].resume
    # case 'x':
    #     syslog(LOG_INFO, "#cTask_raiseException(%d, 0x0001U)", tskno);
    #     SVC_PERROR(cTask_raiseException(tskno, 0x0001U));
    #     break;
    when 120  # x
        p "SampleTask[#{tskno}].raiseException( 0x0001 )"
        SampleTasks[tskno - 1].raiseException( 0x0001 )
    # case 'X':
    #     syslog(LOG_INFO, "#cTask_raiseException(%d, 0x0002U)", tskno);
    #     SVC_PERROR(cTask_raiseException(tskno, 0x0002U));
    #     break;
    when 88   # X
        p "SampleTask[#{tskno}].raiseException( 0x0002 )"
        SampleTasks[tskno - 1].raiseException( 0x0002 )
    # case 'r':
    #     syslog(LOG_INFO, "#rotateReadyQueue(three priorities)");
    #     SVC_PERROR(rotateReadyQueue(HIGH_PRIORITY));
    #     SVC_PERROR(rotateReadyQueue(MID_PRIORITY));
    #     SVC_PERROR(rotateReadyQueue(LOW_PRIORITY));
    #     break;
    when 114  # r
        p "ASPKernel.rotateReadyQueue(three priorities)"
        ASPKernel.rotateReadyQueue( HIGH_PRIORITY )
        ASPKernel.rotateReadyQueue( MID_PRIORITY )
        ASPKernel.rotateReadyQueue( LOW_PRIORITY )
    # case 'c':
    #     syslog(LOG_INFO, "#cCyclic_start(1)");
    #     SVC_PERROR(cCyclic_start());
    #     break;
    when 99   # c
        p "CyclicHandler.start"
        CyclicHandler.start
    # case 'C':
    #     syslog(LOG_INFO, "#cCyclic_stop(1)");
    #     SVC_PERROR(cCyclic_stop());
    #     break;
    when 67   # C
        p "CyclicHandler.stop"
        CyclicHandler.stop
    # case 'b':
    #     syslog(LOG_INFO, "#cAlarm_start(1, 5000)");
    #     SVC_PERROR(cAlarm_start(5000));
    #     break;
    when 98   # b
        p "AlarmHandler.start( 5000 )"
        AlarmHandler.start 5000
    # case 'B':
    #     syslog(LOG_INFO, "#cAlarm_stop()(1)");
    #     SVC_PERROR(cAlarm_stop());
    #     break;
    when 66   # B
        p "AlarmHandler.stop"
        AlarmHandler.stop
    # case 'V':
    # #ifdef TOPPERS_SUPPORT_GET_UTM
    #     SVC_PERROR(getMicroTime(&utime1));
    #     SVC_PERROR(getMicroTime(&utime2));
    #     syslog(LOG_NOTICE, "utime1 = %ld, utime2 = %ld",
    #                                 (ulong_t) utime1, (ulong_t) utime2);
    # #else /* TOPPERS_SUPPORT_GET_UTM */
    #     syslog(LOG_NOTICE, "getMicroTime is not supported.");
    # #endif /* TOPPERS_SUPPORT_GET_UTM */
    #     break;
    when 86   # V
        pUtim1 = TECS::ULongPointer.new(1)
        pUtim2 = TECS::ULongPointer.new(1)
        ASPKernel.getMicroTime(pUtim1)
        ASPKernel.getMicroTime(pUtim2)
        p "utime1 = #{pUtim1[0]}, utime2 = #{pUtim2[0]}"
    # case 'v':
    #     SVC_PERROR(cSysLog_mask(LOG_UPTO(LOG_INFO),
    #                                 LOG_UPTO(LOG_EMERG)));
    #     break;
    when 118  # v
        SysLog.mask( LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG) )

    # case 'q':
    #     SVC_PERROR(cSysLog_mask(LOG_UPTO(LOG_NOTICE),
    #                                 LOG_UPTO(LOG_EMERG)));
    #     break;
    when 113  # q
        SysLog.mask( LOG_UPTO(LOG_NOTICE), LOG_UPTO(LOG_EMERG) )
# #ifdef BIT_KERNEL
    # case ' ':
    #     SVC_PERROR(lockCpu());
    #     {
    #         extern ER	bit_kernel(void);

    #         SVC_PERROR(ercd = bit_kernel());
    #         if (ercd >= 0) {
    #             syslog(LOG_NOTICE, "bit_kernel passed.");
    #         }
    #     }
    #     SVC_PERROR(unlockCpu());
    #     break;
# #endif /* BIT_KERNEL */
    # default:
    #     break;
    # }
# } while (c != '\003' && c != 'Q');
    when 3, 81   # ctrl-C, Q
        break
    end
=begin
# Unused Characters
    when 68   # D
    when 69   # E
    when 70   # F
    when 72   # H
    when 73   # I
    when 74   # J
    when 75   # K
    when 76   # L
    when 77   # M
    when 78   # N
    when 79   # O
    when 80   # P
    when 82   # R
    when 84   # T
    when 85   # U
    when 102  # f
    when 103  # g
    when 104  # h
    when 105  # i
    when 106  # j
    when 107  # k
    when 110  # n
    when 111  # o
    when 112  # p
=end
end   # end of case

# syslog(LOG_NOTICE, "Sample program ends.");
# SVC_PERROR(ciKernel_exitKernel());
# assert(0);

p "Sample program ends."
p "calling exitKernel. This causes skipping mruby final process."
ASPKernel.exitKernel


