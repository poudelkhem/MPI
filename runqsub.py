import subprocess
import time
import sys
#import os.system
#import os.spawn*
#import os.popen*
#import popen2.*
#import commands.*

def main():
    if (len(sys.argv) < 2):
        script = "qtest3.sub"
    else:
        script = sys.argv[1]
    #with open("tout.txt","w") as tfile:
    #    tfile.write("")
    o = subprocess.call(["qsub", script]);
    p = subprocess.Popen(["qstat"], stdout=subprocess.PIPE);
    stdout_data = p.communicate()[0]
    #p1 = subprocess.Popen(["cat"], ["commoutput.txt"], stdout=subprocess.PIPE)
    #catout = p.communicate()[0]
    #with open("tout.txt") as tfile:
    #    content = tfile.read()
    while (stdout_data != ""):
        #print(content)
        print(stdout_data)
        p = subprocess.Popen(["qstat"], stdout=subprocess.PIPE);
        stdout_data = p.communicate()[0]
        #with open("tout.txt") as tfile:
        #    content = tfile.read()
        #p1 = subprocess.Popen(["cat"], ["commoutput.txt"], stdout=subprocess.PIPE)
        #catout = p.communicate()[0]
        time.sleep(3)
        #print(" ")
    print("FINISHED")
                    
                  
                        


main()
