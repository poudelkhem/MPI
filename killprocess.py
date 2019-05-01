import subprocess
import time
import sys
#import os.system
#import os.spawn*
#import os.popen*
#import popen2.*
#import commands.*

def main():
    if (len(sys.argv) > 2):
        
        pid = sys.argv[1]
        process = sys.argv[2]
    elif (len(sys.argv) > 1):
        print("NO pid")
        pid = -1
        process = sys.argv[1]
    else:
        print("need pid andprocessname")
        pid = -1
        process = "prof_main"
    if pid != -1:
        p = subprocess.Popen(["qdel", pid], stdout=subprocess.PIPE);

    #for i in range(0,29):
    #    #ssh compute-1-2 pkill -f main
    #    compute = "compute-1-" + str(i)
    #    print("\nCOMPUTE NODE ================", compute,"\n")
    #    p = subprocess.Popen(["ssh", compute,"pkill", "-f", process], stdout=subprocess.PIPE);
    #    time.sleep(1)
    #    p = subprocess.Popen(["ssh", compute,"ps", "-l"], stdout=subprocess.PIPE);
    #    stdout_data = p.communicate()[0]
    #    print(len(stdout_data.split("\n")))
    #    print(stdout_data);
    for i in range(0,19):
        compute = "compute-0-" + str(i)
        print("\nCOMPUTE NODE ================", compute,"\n")
        p = subprocess.Popen(["ssh", compute,"pkill", "-f", process], stdout=subprocess.PIPE);
        p = subprocess.Popen(["ssh", compute,"ps", "-l"], stdout=subprocess.PIPE);
        time.sleep(1)
        stdout_data = p.communicate()[0]
        print(len(stdout_data.split("\n")))
        print(stdout_data);
    print("FINISHED")
                    
                  
                        


main()
