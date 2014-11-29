
import subprocess
import glob

def run(arg,expected):
    out = subprocess.check_output(["../tab", arg])
    out = out.decode('ascii')
    print(">>>", arg.replace('\n',' '))
    if not expected.startswith(out):
        raise Exception("Test failed for: " + arg)
    
def go():
    l = glob.glob("*.test.in")
    for i in l:
        txt = open(i).read()
        txt = txt.split('===>\n')
        run(txt[0],txt[1])

go()

    
