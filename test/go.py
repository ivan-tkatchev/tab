
import subprocess
import glob

def run(filename,arg,expected, infile = "LICENSE.txt"):
    print(">>>", arg.replace('\n',' '))
    out = subprocess.check_output(["../tab", "-i", "../" + infile, arg])
    out = out.decode('ascii')
    if not expected.startswith(out):
        raise Exception("Test failed for: " + filename + ", " + arg)

def go():
    l = glob.glob("*.test.in")
    for i in l:
        txt = open(i).read()
        txt = txt.split('===>\n')

        if len(txt) == 3:
            run(i,txt[1],txt[2],txt[0].replace('\n',''))
        elif len(txt) == 2:
            run(i,txt[0],txt[1])
        else:
            raise Exception("Malformed test case file: " + i)

go()

    
