
import subprocess
import glob

def run(filename,arg,expected):
    print(">>>", arg.replace('\n',' '))
    out = subprocess.check_output(["../tab", "-i", "../LICENSE.txt", arg])
    out = out.decode('ascii')
    if not expected.startswith(out):
        raise Exception("Test failed for: " + filename + ", " + arg)

def go():
    l = glob.glob("*.test.in")
    for i in l:
        txt = open(i).read()
        txt = txt.split('===>\n')
        run(i,txt[0],txt[1])

go()

    
