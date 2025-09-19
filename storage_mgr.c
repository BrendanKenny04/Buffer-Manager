#include "storage_mgr.h"
#include "dberror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


head* dir;
RC append (head* h, link* l){
    if(h->first){
        link *i = h->first;
        while (i){
        if (i->next)i = i->next;
        else break;
        }
        i->next = l;
    }
    else{
        h->first = l;
    }
    h->length+=1;
    return RC_OK;
}
RC delete (head* h, link* l){
    if(h->first){
        link *i = h->first;
        while (i->next){
            if(i==l){
                h->first=i->next;
                return RC_OK;
            }else if(i->next==l){
                i->next=i->next->next;
                h->length-=1;
                return RC_OK;
            }else i = i->next;
        }
        return RC_FILE_NOT_FOUND;
    }
}

void main(){
    initStorageManager();
    char* f1 = "File One";
    char* f2 = "File Two";
    char* f3 = "File Three";
    createPageFile(f1);
    createPageFile(f2);
    createPageFile(f3);
    SM_FileHandle *fileone = malloc(24);
    SM_FileHandle *filetwo = malloc(24);
    openPageFile(f1,fileone);
    openPageFile(f2,filetwo);


    SM_PageHandle memone = malloc(PAGE_SIZE);
    SM_PageHandle memtwo = malloc(PAGE_SIZE);
    for (int i=0;i<PAGE_SIZE;i++){
        sprintf(memone+i,"%x",i%16);
        sprintf(memtwo+i,"%d",i%2);
    }
    ensureCapacity(2,filetwo);

    writeBlock(0,filetwo,memone);
    writeBlock(1,filetwo,memtwo);


    closePageFile(fileone);
    closePageFile(filetwo);
    destroyPageFile(f3);
    destroyPageFile(f2);
    destroyPageFile(f1);
}
void initStorageManager (void){
    dir = calloc(1,DIR_SIZE);
    dir->first=NULL;
    dir->length=0;
}
RC createPageFile (char *fileName){
    SM_FileHandle *new = malloc(24);
    new->fileName=fileName;
    new->totalNumPages=1;
    new->curPagePos=0;
    link *page = malloc(16);
    page->next = NULL;
    page->data = calloc(1,PAGE_SIZE);
    head *plist = calloc(1,DIR_SIZE);
    append(plist,page);
    new->mgmtInfo = plist;
    link *file = malloc(16);
    file->next = NULL;
    file->data = new;
    append(dir,file);
    return RC_OK;
}
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    if(!dir->first) return RC_FILE_NOT_FOUND;
    if(!fHandle) return RC_FILE_HANDLE_NOT_INIT;
    link *i = dir->first;
    do{
        SM_FileHandle *f = i->data;
        if(strcmp(f->fileName,fileName)==0){
            fHandle->fileName=f->fileName;
            fHandle->curPagePos=f->curPagePos;
            fHandle->totalNumPages=f->totalNumPages;
            fHandle->mgmtInfo=f->mgmtInfo;
            return RC_OK;
        }else i = i->next;
    }while(i);
    return RC_FILE_NOT_FOUND;
}
RC closePageFile (SM_FileHandle *fHandle){
    if(fHandle) {
        free(fHandle);
        return RC_OK;
    }else return RC_FILE_HANDLE_NOT_INIT;
}
RC destroyPageFile (char *fileName){
    if(!dir->first) return RC_FILE_NOT_FOUND;
    link *i = dir->first;
    do{
        SM_FileHandle *f = i->data;
        if(strcmp(f->fileName,fileName)==0){
            head *p = f->mgmtInfo;
            link *j = p->first;
            do{
                free(j->data);
                if(j->next){
                    link *tmp = j->next;
                    free(j);
                    j=tmp;
                }else {
                    free(j);  
                    break;
                }
            }while(1);
            free(f->mgmtInfo);
            //printf("freed %s\n",f->fileName);
            free(f);
            
            return RC_OK;
        }else i = i->next;
    }while(i);
    return RC_FILE_NOT_FOUND;
}
/* reading blocks from disc */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    head *pagelist = fHandle->mgmtInfo;
    if(pageNum<0 || pageNum>=pagelist->length) return RC_READ_NON_EXISTING_PAGE;
    link *i = pagelist->first;
    for(int k;k<pageNum;k++){
        if(i->next)i=i->next;
    }
    char* page = i->data;
    strncpy(memPage,page,PAGE_SIZE);
    return RC_OK;
}
int getBlockPos (SM_FileHandle *fHandle){
    return fHandle->curPagePos;
}
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(0,fHandle,memPage);
}
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(--(fHandle->curPagePos),fHandle,memPage);
}
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(fHandle->curPagePos,fHandle,memPage);
}
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return readBlock(++(fHandle->curPagePos),fHandle,memPage);
}
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    head* p = fHandle->mgmtInfo;
    return readBlock((p->length)-1,fHandle,memPage);
}
/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    head *pagelist = fHandle->mgmtInfo;
    if(pageNum<0 || pageNum>=pagelist->length) return RC_READ_NON_EXISTING_PAGE;
    link *i = pagelist->first;
    for(pageNum;pageNum>0;pageNum--){
        if(i->next)i=i->next;
    }
    char* page = i->data;
    strncpy(page,memPage,PAGE_SIZE);
    return RC_OK;
}
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    return writeBlock(fHandle->curPagePos,fHandle,memPage);
}
RC appendEmptyBlock (SM_FileHandle *fHandle){
    head* pl = fHandle->mgmtInfo;
    void* new = calloc(1,PAGE_SIZE);
    link* l = malloc(16);
    l->data=new;
    RC err = append(pl,l);
    if(!err) fHandle->totalNumPages+=1;
    return err;
}
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    head* pl = fHandle->mgmtInfo;
    int err =0;
    while(numberOfPages>pl->length&&!err){
        err+=appendEmptyBlock(fHandle);
    }return err;
}