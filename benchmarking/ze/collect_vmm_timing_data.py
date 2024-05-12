import os
import time

PAGE_SIZE = 1 << 21
ALLOCATION_SIZES = [i * (PAGE_SIZE) for i in range(1, 8193)]
N_REPEAT = 10
OUTPUT_FILENAME = "ze_vmm_timing_data.csv"

def main():

	total_cnt = N_REPEAT * len(ALLOCATION_SIZES)
	next_pct = 1

	## sleep for 1 second after every timing run for control reasons...
	sleep_time = 1

	cnt = 0
	for size in ALLOCATION_SIZES:
		# Repeat each allocation size multiple times to account for variance
		for repeat in range(N_REPEAT):

			# The exectuable accepts an allocation size and prints the timing results a 1 line to csv file
			# All timings are in ns
			# The format of printed csv line is:
			# 	1. page-aligned allocation size bytes, 
			#	2. Time for phys mem create
			# 	3. Time for reserving va space
			#	4. Time for memory mapping
			#	5. Time for setting access
			#	6. Time for unmapping
			#	7. Time for free va space
			#	8. Time for releasing the original phys mem handle, which then creates free phys space
			os.system("./ze_memMapTest " + str(size) + " >> " + OUTPUT_FILENAME)
			cnt += 1
			if (100 * (cnt / total_cnt)) >= next_pct:
				print("Progress: " + str(next_pct) + "%")
				next_pct += 1
			time.sleep(sleep_time)



if __name__ == "__main__":
    main()