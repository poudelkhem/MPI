import subprocess
import time
#import os.system
#import os.spawn*
#import os.popen*
#import popen2.*
#import commands.*

def main():
    with open("time.txt", "w") as ot:
        ot.write("DataPoints;Nodes;Targets;readPoints;getLocalHead;buildGlobalTree;readTargets;getSendSize;buildLocalTree;sendSendSize;assignTargets;localCount;globalCount;totalTime\n")
    open("error.txt", "w").close()
    numtest = 1
    targets = 20000000
    cpu = "intel.q" #"amd8.q" #  qconf -sql
    first = 32
    ranks = []
    rankIncrements = 2
    for i in range(rankIncrements+1):
        ranks.append(first)
        first = first*2
        #ranks = [i for i in range(8] #,16]
    
    ranks = [16, 32, 64, 128]
    dpranks = [[10000,50000],[10000,50000,100000],[10000,50000,100000],[50000,100000]]
    ranks = [128]
    #dpranks = [[1000000,10000000,100000000,500000000]]
    dpranks = [[100000000]] #[[100000000]]#, 500000000, 1000000000]]
    multi = -1
    #mult = [1,2,5]
    for rank in ranks:
        multi += 1
        #mult = min(int(rank/2),10)
        #datapoints = [i for i in range(10000000,mult[multi]*20000000 + 1,20000000)] #[40] #, 80, 160]
        datapoints = dpranks[multi]
        for datapoint in datapoints:
            with open("error.txt", "a") as ot:
                ot.write("\n\n============================\nNEWRUN ")
                ot.write(str(rank) + " " + str(datapoint) + "\n==============================\n\n")
        
            testArray = [0.0 for i in range(6)] 
            #my_file = open("test.txt", "a").write("")
            for k in range(numtest):
                print("datapoint:  ", datapoint, "num_ranks:  ", rank )
                with open("time.txt", "a") as ot:
                    ot.write(str(datapoint)+ ";" + str(rank)+ ";" + str(targets))
                with open("qtest3.sub", "w") as qt:
                    qt.write("!#/bin/bash\n"


                             "#$ -N main\n"
                             "#$ -pe mpi " + str(rank) + "\n"
                             "#$ -q "+cpu+"\n"
                             
                             
                             "mpirun -np $NSLOTS $HOME/Final/prof_main " + str(datapoint) + " " + str(targets) + " 2>> $HOME/Final/error.txt 1>> $HOME/Final/time.txt  \n")
   
                start = time.time()
                o = subprocess.call(["qsub", "qtest3.sub"]);
                p = subprocess.Popen(["qstat"], stdout=subprocess.PIPE);
                stdout_data = p.communicate()[0]
                while (stdout_data != ""):
                    #print(stdout_data)
                    p = subprocess.Popen(["qstat"], stdout=subprocess.PIPE);
                    stdout_data = p.communicate()[0]
                    end = time.time()
                    print(rank,datapoint,end-start)
                    time.sleep(3)
                    #print(" ")
    

            #with open("test.txt", "r") as file2:
            #    #file2.readline()
            #    allLines = file2.readlines() #.split(",")
            #for line in allLines:
            #    temp = line.split(";")
            #    for j in range(2,8):
            #      testArray[j-2] += float(temp[j])
            #with open("output.txt", "a") as file2:
            #    file2.write(str(datapoint) + ";" + str(rank))
            #    for el in testArray:
            #      file2.write(";" + str(el/numtest))
            #    file2.write("\n")
                
                  
                        


main()
