#ifndef _FCNTL_H
#define _FCNTL_H

#define O_RDONLY 1 //只读
#define O_WRONLY 2 //只写
#define O_RDWR 3 //读写

#define O_APPEND 1
#define O_CREAT 2 //如果指定文件不存在，则创建这个文件
#define O_EXCL 3 //如果要创建的文件已存在，则返回 -1，并且修改 errno 的值,用于测试文件是否存在
#define O_TRUNC 4 //如果文件存在，并且以只写/读写方式打开，则清空文件全部内容 
#define O_NOCTTY 5 //如果路径名指向终端设备，不要把这个设备用作控制终端
#define O_NONBLOCK 6 //如果路径名指向 FIFO/块文件/字符文件，则把文件的打开和后继 I/O 设置为非阻塞模式（nonblocking mode）


#endif