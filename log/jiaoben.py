#!/usr/bin/python3
import sys
import os

def TarLogFile(filename):
    # print("start jiaoben.py")
    arr = filename.split('.')[:-1]
    s = '.'
    frontname = s.join(arr)
    tarname   = frontname + ".tar"

    # print("tar file name:", tarfile)
    if os.path.exists(tarname):
        # print("解压缩包")
        # 解压缩包
        cmd = "tar -zxf " + tarname
        os.system(cmd)
        cmd = "rm " + tarname
        os.system(cmd)

        cmd = "ls " + frontname + ".log*"
        lines = os.popen(cmd).readlines()
        lines.reverse()
        # 重命名
        for line in lines:
            line = line.strip()
            newname = line[:-1] + str(int(line[-1]) + 1)
            cmd = "mv " + line + " " + newname
            os.system(cmd)
        # 重新打包
        cmd = "tar -zcf " + tarname + " " + frontname + ".log*"
        os.system(cmd)
        # 删除未打入包的日志
        cmd = "rm " + frontname + ".log*"
        # print(cmd)
        os.system(cmd)
    else:
        # cmd = "ls " + frontname + ".log*" + " 2>/dev/null"
        # ret = os.system(cmd)
        # if ret != 512: # file 存在
            # print("建压缩包")
        cmd = "tar -zcf " + tarname + " " + frontname + ".log*" + " 2>/dev/null"
        os.system(cmd)
