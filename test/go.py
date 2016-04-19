
import sys
import subprocess
import glob
import struct
import time

def exec(*popenargs, **kwargs):
    proctime = time.time()
    with subprocess.Popen(*popenargs, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs) as process:
        try:
            output, err = process.communicate()
        except:
            process.kill()
            process.wait()
            raise
        retcode = process.poll()
    return retcode, output, err, time.time() - proctime

def run(filename, arg, expected, log, infile = "../LICENSE.txt", errcode = 0, sort=False):
    print(">>>", arg.replace('\n',' '))

    threads = (arg.find("-->") >= 0)

    retcode, out, err, proctime = exec(["../tab", "-r", "1234", "-i", infile, arg] +
                                       (["-s"] if sort else []) +
                                       (["-t99"] if threads else []))
    log[filename] = proctime

    if errcode != retcode:
        raise Exception("Test failed for: %s, '%s' -- return code %d" % (filename, arg, retcode))
    out = out.decode('ascii')
    err = err.decode('ascii')
    if errcode != 0:
        out, err = err, out
    if not expected.startswith(out):
        raise Exception("Test failed for: %s, '%s' -- output is '%s'" % (filename, arg, out))
    if not len(err) == 0:
        raise Exception("Test failed for: %s, '%s' -- stderr is '%s'" % (filename, arg, err))

def go(sort=False):
    wordsize = len(struct.pack("@L",0))
    l = glob.glob("*.test.in") + glob.glob("*.test64.in" if wordsize >= 8 else "*.test32.in")
    log = {}
    for i in l:
        txt = open(i).read()
        txt = txt.split('===>\n')

        if len(txt) == 3:
            run(i, txt[1], txt[2], log, infile=txt[0].replace('\n',''), sort=sort)
        elif len(txt) == 2:
            run(i, txt[0], txt[1], log, sort=sort)
        elif len(txt) == 1:
            txt = txt[0].split('!!!\n')
            run(i, txt[0], txt[1], log, errcode=1, sort=sort)
        else:
            raise Exception("Malformed test case file: " + i)

    l = list(reversed(sorted([ (y,x) for x,y in log.items() ])))[:10]
    total = sum(( y for x,y in log.items() ))
    for t,n in l:
        print('%g\t%s' % (t,n))
    print('Total time: %g' % total)

go(sort=("-s" in sys.argv))

    
