
import subprocess
import glob
import struct

def exec(*popenargs, **kwargs):
    with subprocess.Popen(*popenargs, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs) as process:
        try:
            output, err = process.communicate()
        except:
            process.kill()
            process.wait()
            raise
        retcode = process.poll()
    return retcode, output, err

def run(filename, arg, expected, infile = "LICENSE.txt", errcode = 0):
    print(">>>", arg.replace('\n',' '))
    retcode, out, err = exec(["../tab", "-i", "../" + infile, arg])
    if errcode != retcode:
        raise Exception("Test failed for: " + filename + ", " + arg)
    out = out.decode('ascii')
    err = err.decode('ascii')
    if errcode != 0:
        out, err = err, out
    if not expected.startswith(out):
        raise Exception("Test failed for: " + filename + ", " + arg)
    if not len(err) == 0:
        raise Exception("Test failed for: " + filename + ", " + arg)

def go():
    wordsize = len(struct.pack("@L",0))
    l = glob.glob("*.test.in") + glob.glob("*.test64.in" if wordsize >= 8 else "*.test32.in")
    for i in l:
        txt = open(i).read()
        txt = txt.split('===>\n')

        if len(txt) == 3:
            run(i, txt[1], txt[2], infile=txt[0].replace('\n',''))
        elif len(txt) == 2:
            run(i, txt[0], txt[1])
        elif len(txt) == 1:
            txt = txt[0].split('!!!\n')
            run(i, txt[0], txt[1], errcode=1)
        else:
            raise Exception("Malformed test case file: " + i)

go()

    
