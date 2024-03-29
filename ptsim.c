#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384 // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8 // Shift page number this much

#define PTP_OFFSET 64 // How far offset in page 0 is the page table pointer table

// Simulated RAM
unsigned char mem[MEM_SIZE];

//
// Convert a page,offset into an address
//
int get_address(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

//
// Initialize RAM
//
void initialize_mem(void)
{
    memset(mem, 0, MEM_SIZE);

    int zpfree_addr = get_address(0, 0);
    mem[zpfree_addr] = 1; // Mark zero page as allocated
}

//
// Get the page table page for a given process
//
unsigned char get_page_table(int proc_num)
{
    int ptp_addr = get_address(0, PTP_OFFSET + proc_num);
    return mem[ptp_addr];
}

//
int allocated_pages = 0; // Variable to keep track of allocated pages

int allocate_a_page()
{
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        if (mem[i] == 0)
        {
            mem[i] = 1;
            allocated_pages++; // Increment allocated_pages
            return i; // Return the allocated page number
        }
    }
    return -1; // Return -1 if no page is available
}

// Allocate pages for a new process
//
// This includes the new process page table and page_count data pages.
//
void new_process(int proc_num, int page_count)
{
    // inital page
    int page_table = allocate_a_page();

    // checks initial page
    if (page_table == -1)
    {
        printf("OOM: proc %d: page table\n", proc_num);
        return;
    }

    // Allocate the requested data pages and update page table entries
    mem[PTP_OFFSET + proc_num] = page_table;

    for (int i = 0; i < page_count; i++)
    {
        int new_page = allocate_a_page();
        if (new_page == -1)
        {
            printf("OOM: proc %d: data page\n", proc_num);
            return;
        }
        int page_table_entry = get_address(page_table, i);
        mem[page_table_entry] = new_page;
    }
}


void kill_process(int proc_num)
{
    // Get the page table for the process
    int page_table = get_page_table(proc_num);
 

    // Free all the pages listed in the page table
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        int page_table_entry = get_address(page_table, i);
        int page = mem[page_table_entry];

        if (page != 0)
        {
            mem[page] = 0; // Free the page
        }
    }

    // Free the page table itself
    mem[page_table]=0;
    mem[PTP_OFFSET + proc_num] = 0;
}



//
// Print the free page map
//
// Don't modify this
//
void print_page_free_map(void)
{
    printf("--- PAGE FREE MAP ---\n");

    for (int i = 0; i < 64; i++)
    {
        int addr = get_address(0, i);

        printf("%c", mem[addr] == 0 ? '.' : '#');

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

//
// Print the address map from virtual pages to physical
//
// Don't modify this
//
void print_page_table(int proc_num)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", proc_num);

    // Get the page table for this process
    int page_table = get_page_table(proc_num);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        int addr = get_address(page_table, i);

        int page = mem[addr];

        if (page != 0)
        {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

int vert_to_phys(int proc_num, int vaddr){
    int page_table = get_page_table(proc_num);

    // Calculate the page number and offset
    int page_num = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;

    // Get the physical address from the page table
    int page_table_entry = get_address(page_table, page_num);
    int physical_addr = mem[page_table_entry];

    // Calculate the physical address
    int physical_page = physical_addr * PAGE_SIZE + offset;
    return physical_page;
}
void store_value(int proc_num, int vaddr, int value)
{
    // Get the page table for the process
    int physical_page= vert_to_phys(proc_num, vaddr);

    // Store the value at the physical address
    mem[physical_page] = value;

    // Print the output
    printf("Store proc %d: %d => %d, value=%d\n", proc_num, vaddr, physical_page, value);
}
void load_value(int proc_num, int vaddr){

   
    // Calculate the physical address
    int physical_page= vert_to_phys(proc_num, vaddr);

    // Store the value at the physical address
    int value = mem[physical_page] ;

    // Print the output
    printf("Store proc %d: %d => %d, value=%d\n", proc_num, vaddr, physical_page, value);
}

//
// Main -- process command line
//
int main(int argc, char *argv[])
{
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);

    if (argc == 1)
    {
        fprintf(stderr, "usage: ptsim commands\n");
        return 1;
    }

    initialize_mem();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "pfm") == 0)
        {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0)
        {
            int proc_num = atoi(argv[++i]);
            print_page_table(proc_num);
        }
        else if (strcmp(argv[i], "np") == 0)
        {
            int proc_num = atoi(argv[++i]);
            int page_count = atoi(argv[++i]);
            new_process(proc_num, page_count);
        }
        else if (strcmp(argv[i], "kp") == 0)
        {
            int proc_num = atoi(argv[++i]);
            kill_process(proc_num);

        }
        else if (strcmp(argv[i], "sb") == 0)
        {
            int proc_num= atoi(argv[++i]);
            int addr= atoi(argv[++i]);
            int value =atoi(argv[++i]);
            store_value(proc_num ,addr ,value);


        }

        else if (strcmp(argv[i], "lb") == 0)
        {
            int proc_num= atoi(argv[++i]);
            int addr= atoi(argv[++i]);
            load_value(proc_num ,addr );


        }
    }
}
