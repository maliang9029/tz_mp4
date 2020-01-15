#ifndef _VORX_ERROR_CODE_DEFINE_HEAD_FILE
#define _VORX_ERROR_CODE_DEFINE_HEAD_FILE

namespace vfc
{
#ifndef ERRCODE_BASE
	#define ERRCODE_BASE 0
#endif // !defined ERRCODE_BASE
	
	enum VORX_ERRCODE
	{
		ERR_SUCCESS		= 0,		//执行成功
/*
		ERR_UNKNOWN		= ERRCODE_BASE+1,		//未知错误
		ERR_UNEXIST		= ERRCODE_BASE+2,		//设备不存在
		ERR_NETSEND		= ERRCODE_BASE+3,		//网络发送失败
		ERR_NETTIMEOUT	= ERRCODE_BASE+4,		//网络接收回应包超时
		ERR_UNKNOWNCMD	= ERRCODE_BASE+5,		//未知命令
		ERR_UNLOGINED	= ERRCODE_BASE+6,		//终端未登录
		ERR_UNSUPPORT	= ERRCODE_BASE+7,		//不支持的指令
		ERR_SPACE		= ERRCODE_BASE+8,		//缓冲区空间不足
		ERR_INSTANCE	= ERRCODE_BASE+9,		//实例不存在
		ERR_UNINITIALIZE= ERRCODE_BASE+10,		//未初始化
		ERR_FORMAT		= ERRCODE_BASE+11,		//包格式错误
		ERR_PARAM		= ERRCODE_BASE+12,		//参数错误
		ERR_DEVICE		= ERRCODE_BASE+13,		//删除列表时,目标设备未找到
		ERR_TARGET		= ERRCODE_BASE+14		//删除列表时,目标地址未找到
*/
		// 网络通信
		ERR_NETSEND		= ERRCODE_BASE+0x0101,	//网络发送失败
		ERR_PROTO		= ERRCODE_BASE+0x0102,	//协议格式错误
		ERR_UNSUPPORT	= ERRCODE_BASE+0x0103,	//未支持的指令
		ERR_NETTIMEOUT	= ERRCODE_BASE+0x0104,	//网络应答超时
		ERR_FORMAT		= ERRCODE_BASE+0x0105,	//包格式错误
		ERR_NETINIT		= ERRCODE_BASE+0x0106,	//打开通信网络失败
		ERR_UNKNOWNCMD	= ERRCODE_BASE+0x0107,	//未知命令
		// 数据库操作
		ERR_CONNECTDB	= ERRCODE_BASE+0x0201,	//连接数据库失败
		ERR_UNCONNECTDB	= ERRCODE_BASE+0x0202,	//未连接数据库
		ERR_READDB		= ERRCODE_BASE+0x0203,	//读取数据库数据失败
		ERR_MODIFYDB	= ERRCODE_BASE+0x0204,	//写数据库失败
		// 内存
		ERR_MEMORY		= ERRCODE_BASE+0x0301,	//申请内存失败
		ERR_SPACE		= ERRCODE_BASE+0x0302,	//缓冲区空间不足
		// 组件
		ERR_UNLOGINED	= ERRCODE_BASE+0x0401,	//终端未登录
		ERR_UNINITIALIZE= ERRCODE_BASE+0x0402,	//未初始化
		ERR_INSTANCE	= ERRCODE_BASE+0x0403,	//实例不存在
		ERR_EXISTINSTANCE=ERRCODE_BASE+0x0404,	//实例已存在
		ERR_PARAM		= ERRCODE_BASE+0x0405,	//参数错误
		ERR_DEVICE		= ERRCODE_BASE+0x0406,	//删除列表时,目标设备未找到
		ERR_TARGET		= ERRCODE_BASE+0x0407,	//删除列表时,目标地址未找到
		ERR_UNEXIST		= ERRCODE_BASE+0x0408,	//设备不存在
		// 线程/进程
		ERR_THREAD		= ERRCODE_BASE+0x0501,	//线程错误
		// 文件操作
		ERR_CREATEDIR	= ERRCODE_BASE+0x0601,	//创建目录失败
		ERR_DELDIR		= ERRCODE_BASE+0x0602,	//删除目录失败
		ERR_OPENDIR		= ERRCODE_BASE+0x0603,	//打开目录失败
		ERR_CREATEFILE	= ERRCODE_BASE+0x0604,	//创建文件失败
		ERR_DELLFILE	= ERRCODE_BASE+0x0605,	//删除文件失败
		ERR_OPENFILE	= ERRCODE_BASE+0x0606,	//打开文件失败
		ERR_WRITEFILE	= ERRCODE_BASE+0x0607,	//写文件失败
		ERR_CLOSEFILE	= ERRCODE_BASE+0x0608,	//关闭文件失败
		ERR_SRCTYPEUNDEF= ERRCODE_BASE+0x0609,	//stream中的m_nSrcType为-1
	};

	struct VorxErrDesc
	{
		int nCode;
		char szDesc[50];
	};

/*
	const VorxErrDesc g_errdesc[] = 
	{
		{ERR_SUCCESS, "成功"},
		{ERR_UNKNOWN, "未知错误"},
		{ERR_UNEXIST, "设备不存在"},
		{ERR_NETSEND, "网络发送失败"},
		{ERR_NETTIMEOUT, "网络接收回应包超时"},
		{ERR_UNKNOWNCMD, "未知命令"},
		{ERR_UNLOGINED, "终端未登录"},
		{ERR_UNSUPPORT, "不支持的指令"},
		{ERR_SPACE, "缓冲区空间不足"},
		{ERR_INSTANCE, "实例不存在"},
		{ERR_UNINITIALIZE, "未初始化"},
		{ERR_FORMAT, "包格式错误"},
		{ERR_PARAM, "参数错误"},
		{ERR_DEVICE, "删除列表时,目标设备未找到"},
		{ERR_TARGET, "删除列表时,目标地址未找到"}
	};
*/
	const VorxErrDesc g_errdesc[] = 
	{
		{ERR_SUCCESS, "成功"},
// 		{ERR_UNKNOWN, "未知错误"},
		// 网络通信
		{ERR_NETSEND, "网络发送失败"},
		{ERR_PROTO, "协议格式错误"},
		{ERR_UNSUPPORT, "不支持的指令"},
		{ERR_NETTIMEOUT, "网络接收回应包超时"},
		{ERR_FORMAT, "包格式错误"},
		{ERR_NETINIT, "打开网络失败"},
		{ERR_UNKNOWNCMD, "未知命令"},
		// 数据库操作
		{ERR_CONNECTDB, "连接数据库失败"},
		{ERR_UNCONNECTDB, "未连接数据库"},
		{ERR_READDB, "读取数据库数据失败"},
		{ERR_MODIFYDB, "写数据库数据失败"},
		// 内存
		{ERR_MEMORY, "申请内存失败"},
		{ERR_SPACE, "缓冲区空间不足"},
		// 组件
		{ERR_UNLOGINED, "终端未登录"},
		{ERR_UNINITIALIZE, "未初始化"},
		{ERR_INSTANCE, "实例不存在"},
		{ERR_EXISTINSTANCE, "实例已存在"},
		{ERR_PARAM, "参数错误"},
		{ERR_DEVICE, "删除列表时,目标设备未找到"},
		{ERR_TARGET, "删除列表时,目标地址未找到"},
		{ERR_UNEXIST, "设备不存在"},
		// 线程/进程
		{ERR_THREAD, "线程错误"},
		// 文件操作
		{ERR_CREATEDIR, "创建目录失败"},
		{ERR_DELDIR, "删除目录失败"},
		{ERR_OPENDIR, "打开目录失败"},
		{ERR_CREATEFILE, "创建文件失败"},
		{ERR_DELLFILE, "删除文件失败"},
		{ERR_OPENFILE, "打开文件失败"},
		{ERR_WRITEFILE, "保存文件失败"},
		{ERR_CLOSEFILE, "关闭文件失败"}
	};
}

#endif
