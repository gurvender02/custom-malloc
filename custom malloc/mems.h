/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>

#define PAGE_SIZE 4096


/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
int virtualCounter = 1000;

// creating subListNode
typedef struct slist
{
    struct slist *next;
    void *adrStoringForMemsMellocRequest;
    int isprocces; // what happens when you write malloc in front of variable Linked list
    int sizeStored;
    int vStart;
    int vEnd;

} subList;

typedef struct mList
{
    struct mList *next;
    subList *subListNext;

    void *maxLimitForSubPage;
    void *freeSpaceStartforMemeRequest;

    void *maxLimitToAllocatInMainPage;
    void *freeSpaceStartForNodeAllocation;

    int StartingOfVirtualSpaceforThisNode;
    int maxLimitForVirtualAdressStoredInMainNode;
    int freeSpaceVirtualSpace;

    size_t PagesInMainNode;

} mainList;

mainList *mhead = NULL;
subList *shead = NULL;


/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init()
{
    mhead = (mainList *)mmap(NULL, sizeof(mainList), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    shead = (subList *)(mhead + 1);
    mhead->maxLimitToAllocatInMainPage = (mhead->freeSpaceStartforMemeRequest) + (PAGE_SIZE - 1);
    mhead->freeSpaceStartForNodeAllocation = (void *)(shead + 1);
    mhead->subListNext = shead;
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void unmpping(mainList * node){
    if(node!=NULL && node->next==NULL){
        munmap(node->subListNext->adrStoringForMemsMellocRequest,node->maxLimitForSubPage);
    }
    if(node!=NULL){
        unmpping(node->next);
    }
}   

void mems_finish(){
    mainList *dummyHead = mhead;
    unmpping(dummyHead);
    printf("Successfully umppaded memory\n");
}


/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
void *mems_malloc(size_t size)
{   
    // size_t countPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    mainList *mDummy = mhead;
    subList *sDummy = mDummy->subListNext;
    if (sDummy->isprocces == 0)
    {
        mDummy->freeSpaceStartforMemeRequest = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        mDummy->maxLimitForSubPage = (void *)((char *)mDummy->freeSpaceStartforMemeRequest + PAGE_SIZE - 1);

        sDummy->adrStoringForMemsMellocRequest = mDummy->freeSpaceStartforMemeRequest;
        mDummy->freeSpaceStartforMemeRequest = (void *)((char *)mDummy->freeSpaceStartforMemeRequest + size + 1);
        sDummy->isprocces = 1;
        sDummy->sizeStored = size;
        sDummy->next = NULL;


        mDummy->freeSpaceVirtualSpace = virtualCounter;
        mDummy->maxLimitForVirtualAdressStoredInMainNode = virtualCounter + PAGE_SIZE-1;

        sDummy->vStart = virtualCounter;
        virtualCounter += size;
        sDummy->vEnd = virtualCounter-1;

        mDummy->PagesInMainNode = 1;

        // printf("Physical Addr : %lu\n" , sDummy->adrStoringForMemsMellocRequest);
        return (void*)sDummy->vStart;
    }
    
    mDummy = mhead;
    while (mDummy != NULL)
    {       sDummy = mDummy->subListNext;
        while (sDummy != NULL)
        {   
            if(sDummy->isprocces ==0 && sDummy->sizeStored == size){
                sDummy->isprocces =1;
                return (void*)sDummy->vStart;
            }

            if(sDummy->isprocces == 0 && sDummy->sizeStored > size && sDummy->next!=NULL){
                // printf("spliting a node in to hole and process");
                subList *tempStoringNextNodeofSDummy = sDummy->next;

                subList *hole = (subList*)mDummy->freeSpaceStartForNodeAllocation;
                mDummy->freeSpaceStartForNodeAllocation = (void *)((char *)mDummy->freeSpaceStartForNodeAllocation + sizeof(mainList));   

                hole->isprocces =0;
                hole->next = tempStoringNextNodeofSDummy;
                hole->vStart =sDummy->vStart+size;
                hole->vEnd = sDummy->vEnd;
                
                sDummy->isprocces =1;
                sDummy->next = hole;
                sDummy->sizeStored = size;
                sDummy->vEnd = (sDummy->vStart + size) -1;

            }

            if (sDummy->next == NULL && (((char *)mDummy->freeSpaceStartforMemeRequest + size) <= (char *)mDummy->maxLimitForSubPage))
            {
                // Allocate a new subList node
                // printf("SubNode \n");
                subList * newSubNode = NULL;
                newSubNode =(subList*) mDummy->freeSpaceStartForNodeAllocation ;
                mDummy->freeSpaceStartForNodeAllocation = (void *)((char *)mDummy->freeSpaceStartForNodeAllocation + sizeof(subList));
                
                sDummy->next = newSubNode;
                // // newSubNode->adrStoringForMemsMellocRequest = mDummy->freeSpaceStartforMemeRequest; // Assign the address
                // printf("addresMDummy %lu\n", mDummy->freeSpaceStartforMemeRequest );
                // printf("maxLimit to of mDummy to store malloc Request : %lu \n", mDummy->maxLimitForSubPage);
                // printf("addressof mDummy_fressSpaceStartForMemeest");

                newSubNode->adrStoringForMemsMellocRequest = mDummy->freeSpaceStartforMemeRequest;

                newSubNode->isprocces = 1;
                newSubNode->next = NULL;
                newSubNode->sizeStored = size;

                newSubNode->vStart = virtualCounter;
                virtualCounter+=size;
                newSubNode->vEnd = virtualCounter-1;
                // Update the mainList's freeSpaceStartforMemeRequest
                mDummy->freeSpaceStartforMemeRequest = (void *)((char *)mDummy->freeSpaceStartforMemeRequest + size);

                
                // printf("Physical Addr : %lu\n" , newSubNode->adrStoringForMemsMellocRequest);
                return (void*)newSubNode->vStart;
            }
            sDummy = sDummy->next;
        }
        mDummy = mDummy->next;
    }

    mainList * mDummy2 = mhead; 
    subList * sDummy2 = mhead->subListNext;
    while(mDummy2->next!=NULL){
        mDummy2 = mDummy2->next;
    }

    //time to create a main Node
   
    if(mDummy2->freeSpaceStartForNodeAllocation + sizeof(mainList) + sizeof(subList) <= mDummy2->maxLimitToAllocatInMainPage){
        // printf(" from the prov main Node address free start%lu\n", mDummy2->freeSpaceStartforMemeRequest);
        // printf("page limit %lu\n", mDummy2->maxLimitForSubPage);

         // Calculate the number of pages needed for the new memory allocation
            size_t countPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;


        mainList *newMainNode = (mainList *)mDummy2->freeSpaceStartForNodeAllocation;
        mDummy2->freeSpaceStartForNodeAllocation = (void *)((char *)mDummy2->freeSpaceStartForNodeAllocation + sizeof(mainList));
        
        newMainNode->StartingOfVirtualSpaceforThisNode = virtualCounter;
        newMainNode->maxLimitForVirtualAdressStoredInMainNode = virtualCounter + (countPages*PAGE_SIZE);

        newMainNode->PagesInMainNode = countPages;

        subList *newSubNode = (subList *)mDummy2->freeSpaceStartForNodeAllocation;
        mDummy2->freeSpaceStartForNodeAllocation = (void *)((char *)mDummy2->freeSpaceStartForNodeAllocation + sizeof(subList));

        
        // Initialize the new mainList node and subList node
        newMainNode->PagesInMainNode = mDummy2->PagesInMainNode;
        newMainNode->maxLimitToAllocatInMainPage = mDummy2->maxLimitToAllocatInMainPage;
        newMainNode->freeSpaceStartforMemeRequest = mmap(NULL, countPages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        newMainNode->maxLimitForSubPage = (void *)((char *)newMainNode->freeSpaceStartforMemeRequest + (countPages * PAGE_SIZE - 1));
        newMainNode->subListNext = newSubNode;
        newMainNode->freeSpaceStartForNodeAllocation = mDummy2->freeSpaceStartForNodeAllocation;



        newSubNode->adrStoringForMemsMellocRequest = newMainNode->freeSpaceStartforMemeRequest;
        newMainNode->freeSpaceStartforMemeRequest = (void *)((char *)newMainNode->freeSpaceStartforMemeRequest + size);
        newSubNode->isprocces = 1;
        newSubNode->next = NULL;
        newSubNode->sizeStored = size;

        newSubNode->vStart = virtualCounter;
        virtualCounter+=size;
        newSubNode->vEnd =virtualCounter-1;


        //connecting to the main privous node 
        mDummy2->next = newMainNode;

        // printf("Physical Addr : %lu\n" , newSubNode->adrStoringForMemsMellocRequest);
        return (void*)newSubNode->vStart;
    }else {
        
        // Calculate the number of pages needed for the new memory allocation
        size_t countPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

        
        // printf("free start from the prv dummy%lu\n", mDummy2->freeSpaceStartforMemeRequest);
        // printf("page limit %lu\n", mDummy2->maxLimitForSubPage);
        mainList *newMainNode = (mainList*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        newMainNode->maxLimitToAllocatInMainPage = (void*)(newMainNode + countPages*PAGE_SIZE); 
        newMainNode->freeSpaceStartForNodeAllocation = (void*)(newMainNode +1);

        newMainNode->PagesInMainNode = countPages;    

        subList *newSubNode = (subList *)newMainNode->freeSpaceStartForNodeAllocation;
        newMainNode->freeSpaceStartForNodeAllocation = (void *)((char *)newMainNode->freeSpaceStartForNodeAllocation + sizeof(subList));
        
        newMainNode->StartingOfVirtualSpaceforThisNode = virtualCounter;
        newMainNode->maxLimitForVirtualAdressStoredInMainNode = virtualCounter + (countPages*PAGE_SIZE);
    
        
        newMainNode->freeSpaceStartforMemeRequest = mmap(NULL, countPages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        newMainNode->maxLimitForSubPage = (void *)((char *)newMainNode->freeSpaceStartforMemeRequest + (countPages * PAGE_SIZE - 1));
        newMainNode->subListNext = newSubNode;

    

        newSubNode->adrStoringForMemsMellocRequest = newMainNode->freeSpaceStartforMemeRequest;
        newMainNode->freeSpaceStartforMemeRequest = (void *)((char *)newMainNode->freeSpaceStartforMemeRequest + size);
        newSubNode->isprocces = 1;
        newSubNode->next = NULL;
        newSubNode->sizeStored = size;
        newSubNode->vStart = virtualCounter;
        virtualCounter+=size;
        newSubNode->vEnd = virtualCounter-1;

        mDummy2->next = newMainNode;
        
        // printf("Physical Addr : %lu\n" , newSubNode->adrStoringForMemsMellocRequest);
        return (void*)newSubNode->vStart;
    }

    return NULL;
}

//check for margingTow two Conscutive two hole;
void marger(){
    mainList *dummyHeadNode = mhead;
    while(dummyHeadNode!=NULL){
        subList *subDummyNode = dummyHeadNode->subListNext;
        while (subDummyNode!=NULL)
        {
            if(subDummyNode->isprocces ==0 && subDummyNode->next !=NULL && subDummyNode->next->isprocces ==0 ){
                subList *tempSub = subDummyNode->next;

                subDummyNode->sizeStored +=tempSub->sizeStored;
                subDummyNode->vEnd = tempSub->vEnd;
                subDummyNode->next = tempSub->next;

            }

            subDummyNode = subDummyNode->next;
        }
        

        dummyHeadNode = dummyHeadNode->next;
    }
}


/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(){
    marger();
    mainList *dummyHead = mhead;

    int countTotalNumberOfPages = 0;
    int totalSpaceUsed =0;
    int totalMainchainNode =0;

    printf("--------------Mems System State-------------------\n");
    while(dummyHead!=NULL){

        totalMainchainNode++;
        subList *subDummyNode = dummyHead->subListNext;
        countTotalNumberOfPages += dummyHead->PagesInMainNode;


        printf("MAIN[%lu:%lu]->",dummyHead->StartingOfVirtualSpaceforThisNode, dummyHead->maxLimitForVirtualAdressStoredInMainNode);
        while (subDummyNode!=NULL)
        {   
            if(subDummyNode->isprocces ==1){
                totalSpaceUsed += subDummyNode->sizeStored;
                printf("P");
            }else{
                printf("H");
            }
            printf("[%d:%d] <-> ",subDummyNode->vStart, subDummyNode->vEnd);
            subDummyNode= subDummyNode->next;
        }
        printf("NULL\n");

        dummyHead = dummyHead->next;
    }
    printf("Total Pages Used : %d\n", countTotalNumberOfPages);
    printf("total Space Used : %d\n", totalSpaceUsed);
    printf("Main chain Length: %d\n", totalMainchainNode);

    //for printing number of lenght of subList
    dummyHead = mhead;
    printf("sub Chain lenght:");
    while(dummyHead!=NULL){
        subList *subNode = dummyHead->subListNext;
        int numbNodeCounter =0;
        while(subNode!=NULL){

            numbNodeCounter++;
            subNode = subNode->next;
        }
        printf(" %d", numbNodeCounter);
        dummyHead = dummyHead->next;
    }
    printf("\n");

}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void *v_ptr)
{
    int target = (int)v_ptr;
    mainList *dummyHead = mhead;

    void *adrPtrtoTargetLocation = NULL;

    while (dummyHead != NULL)
    {
        if (target >= dummyHead->StartingOfVirtualSpaceforThisNode && target <= dummyHead->maxLimitForVirtualAdressStoredInMainNode)
        {
            // Calculate the offset within the main node's memory range
            size_t offset = target - dummyHead->StartingOfVirtualSpaceforThisNode;

            // Calculate the physical address by adding the offset to the start of the memory range
            adrPtrtoTargetLocation = (void*)((char *)dummyHead->maxLimitForSubPage - offset);

            break;  // Terminate the loop once the address is found
        }

        dummyHead = dummyHead->next;
    }

    return adrPtrtoTargetLocation;
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void *v_ptr){
    
    int target = (int)v_ptr;
    mainList *dummyHead = mhead;

    void *adrPtrtoTargetLocation = NULL;

    while (dummyHead != NULL)
    {
       subList *subNode = dummyHead->subListNext;

       while (subNode!=NULL)
       {
            if(subNode->vStart<=target && subNode->vEnd>= target){
                subNode->isprocces =0;
            }
            subNode = subNode->next;
       }

        dummyHead = dummyHead->next;
    }
}