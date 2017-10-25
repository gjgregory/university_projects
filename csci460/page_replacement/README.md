## Page Replacement

The task was to implement an inverted page table shared by all processes in a demand paging system, as well as two page replacement algorithms: Enhanced Second Chance and Least Frequently Used. Then their respective performance with random page replacements was to be tested, as outlined in the PDF.

* pagerep_esc.c - This implements functions declared in Assg3.h that are used by the Simumlate() function in such a way as to simulate the Enhanced Second Chance page replacement algorithm with an inverted page table.

* pagerep_lfu.c - This file implements functions declared in Assg3.h that are used by the Simumlate() function in such a way as to simulate the Least Frequently Used page replacement algorithm with an inverted page table.

Usage:

```
make clean
make all
./pagerep_esc <numer_of_rounds>
./pagerep_lfu <numer_of_rounds>
```
