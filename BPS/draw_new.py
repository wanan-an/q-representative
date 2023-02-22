import matplotlib.pyplot as plt
import os

filepath1 = "sample_rate_bps.txt"
filepath2 = "sample_rate_swtc.txt"

if __name__ == "__main__":
    with open (filepath1, 'r+') as f1:
        f1.readline()
        rate_bps = list(map(float,f1.readline().rstrip().split(" ")))[1600:-1]
    with open (filepath2, 'r+') as f2:
        time = list(map(int,f2.readline().rstrip().split(" ")))[1600:-1]
        rate_swtc = list(map(float,f2.readline().rstrip().split(" ")))[1600:-1]
    print(rate_bps)
    print(rate_swtc)
    plt.suptitle("window_size: 4000000 stream number:160000")
    plt.xlabel("Processed Time Unit")
    plt.ylabel("Percentage of Valid Sample")
    plt.plot(time, rate_bps)
    plt.plot(time, rate_swtc)
    plt.legend(['BPS','SWTC'])
    plt.savefig("sample_rate.jpg",format="JPG")
    plt.show()

