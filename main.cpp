#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <bits/stdc++.h>
using namespace std;
#define DB 131072        
#define DBlock 4096          
#define NOI 78644        
#define NOFD 32 
struct inode 
{
    int fileSize;
    int pointer[12];
};

struct FTIM 
{
    char fileNameS[32]; 
    int inodeNumber;    
};

struct blockInformation
{
    bool InodeListBool[NOI]; 
    int temp1 = ceil(((float)sizeof(struct FTIM) * NOI) / DBlock);
    int blockInformationSize = ceil(((float)sizeof(blockInformation)) / DBlock);
    int temp2 = ceil(((float)(NOI * sizeof(struct inode))) / DBlock);
    int indexDB = blockInformationSize + temp1 + temp2;
    int inodeIndexStart = blockInformationSize + temp1;
    bool freeListDB[DB];
};
static int active = 0;
char vDiskName[50], filename[30];
struct FTIM inodeMapArray[NOI];
struct blockInformation sb;
struct inode inodeArryList[NOI];
vector<int> inodeFreeV;               
vector<int> fDBVector;  
map<string, int> fileStorInode;          
vector<int> FDVector;       
map<int, pair<int, int>> FDMap; 
int fileCount = 0;  
map<int, int> FDMMap;       
map<int, string> InodeFileStorMap;          
FILE *diskPointer;
int fprintfErrorRead()
{
    if (!active)
    {
        fprintf(stderr, "Block Read Write Error disk is not mounted properly or no open disk\n");
        printf("Block Read Write Error disk is not mounted properly or no open disk\n");
        return -1;
    }
    return 0;
}
int fprintfError(int block)
{
    if ((block >= DB)||(block < 0))
    {
        fprintf(stderr, "Block Write Read Error Block index out of bounds\n");
        return -1;
    }
    return 0;
}
int fseekError(int block,int startIndex)
{
    if (fseek(diskPointer, (block * DBlock) + startIndex, SEEK_SET) < 0)
    {
        perror("Block Write Read Error");
        return -1;
    }
    return 0;
}
int freadError(char *buffer)
{
    if (fread(buffer, sizeof(char), DBlock, diskPointer) < 0)
    {
        perror("Block Read Error");
        return -1;
    }
    return 0;
}
int fwriteError(char *buffer,int size)
{
    if (fwrite(buffer, sizeof(char), size, diskPointer) < 0)
    {
        perror("Block Write Error");
        return -1;
    }
    return 0;
}
int BRead(int block, char *buffer)
{
    if(fprintfErrorRead()==-1)
    {
        return -1;
    }
    if(fprintfError(block)==-1)
    {
        return -1;
    }
    if (fseekError(block,0)==-1)
    {
        return -1;
    }
    if(freadError(buffer)==-1)
    {
        return -1;
    }
    return 0;
}
int BWrite(int block, char *buffer, int size, int startIndex)
{
    if(fprintfError(block)==-1)
    {
        return -1;
    }
    if(fseekError(block,startIndex)==-1)
    {
        return -1;
    }
    if(fwriteError(buffer,size)==-1)
    {
        return -1;
    }
    return 0;
}
void inodePointerErase(int currentPointer)
{
    inodeArryList[currentPointer].fileSize = 0;
    for (int i = 0; i < 12; ++i)
    {
        inodeArryList[currentPointer].pointer[i] = -1;
    }
}
void eraseInodeePointer(int currentPointer,bool *deleteBool)
{
    for (int i = 0; i < 10; i++)
    {
        if (inodeArryList[currentPointer].pointer[i]!= -1)
        {
            fDBVector.push_back(inodeArryList[currentPointer].pointer[i]);
            inodeArryList[currentPointer].pointer[i] = -1;
        }
        else
        {
            *deleteBool = true;
            return;
        }
    }
}
void eraseInodeePointer1(bool *deleteBool,int *iPointerArray)
{
    for (int i = 0; i < 1024; i++)
    {
        if (iPointerArray[i]!= -1)
        {
            fDBVector.push_back(iPointerArray[i]);
        }
        else
        {
            *deleteBool = true;
            return;
        }
    }
}
void earseIndirectPointer(int *iPointerArray,bool *deleteBool)
{
    for (int i = 0; i < 1024; i++)
    {
        if (iPointerArray[i]!= -1)
        {
            fDBVector.push_back(iPointerArray[i]);
        }
        else
        {
            *deleteBool = true;
            return;
        }
    }
}
int deleteIContent(int currentPointer)
{
    bool deleteBool = false;
    eraseInodeePointer(currentPointer,&deleteBool);
    int indirectptr,doubleindirectptr;
    if (deleteBool==false)
    {
        char blockbufferfer[DBlock];
        indirectptr = inodeArryList[currentPointer].pointer[10];
        BRead(indirectptr, blockbufferfer);
        int iPointerArray[1024];
        memcpy(iPointerArray, blockbufferfer, sizeof(iPointerArray));
        eraseInodeePointer1(&deleteBool,iPointerArray);
        inodeArryList[currentPointer].pointer[10] = -1;
        fDBVector.push_back(indirectptr);
    }
    if (deleteBool==false)
    {
        char blockbufferfer[DBlock];
        int doubleiPointerArray[1024];
        doubleindirectptr = inodeArryList[currentPointer].pointer[11];
        BRead(doubleindirectptr, blockbufferfer);
        memcpy(doubleiPointerArray, blockbufferfer, sizeof(doubleiPointerArray));
        for (int i = 0; i < 1024; i++)
        {
            char blockbufferfer1[DBlock];
            int iPointerArray[1024];
            if (doubleiPointerArray[i] != -1)
            {
                int singleindirectptr = doubleiPointerArray[i];
                BRead(singleindirectptr, blockbufferfer1);
                memcpy(iPointerArray, blockbufferfer1, sizeof(iPointerArray));
                earseIndirectPointer(iPointerArray,&deleteBool);
                fDBVector.push_back(singleindirectptr);
            }
            else
            {
                deleteBool = true;
                break;
            }
        }
        fDBVector.push_back(doubleindirectptr);
    }
    inodePointerErase(currentPointer);
    return 0;
}
int file200Error(string filename)
{
    if (fileStorInode.find(filename) != fileStorInode.end())
    {
        printf("File already is in system please try with different filename\n");
        return -1;
    }
    return 0;
}
int fileInodeData()
{
    if (!inodeFreeV.size()||!fDBVector.size())
    {
        return -1;
    }
    return 0;
}
int fileCreateInMem(char *name)
{
    string filename = string(name);
    if(file200Error(filename)==-1)
    {
        return -1;
    }
    if(fileInodeData()==-1)
    {
        printf("Space constraint no Inode/Data Block is avialable\n");
        return -1;
    }
    int nAvailnode = inodeFreeV.back();
    int nAvailnodeDB = fDBVector.back();
    inodeArryList[nAvailnode].fileSize = 0;
    inodeArryList[nAvailnode].pointer[0] = nAvailnodeDB;
    fileStorInode[filename] = nAvailnode;
    inodeMapArray[nAvailnode].inodeNumber = nAvailnode;
    InodeFileStorMap[nAvailnode] = filename;
    strcpy(inodeMapArray[nAvailnode].fileNameS, name);
    inodeFreeV.pop_back();
    fDBVector.pop_back();
    printf("File created Sucessfully\n");
    return 1;
}
int file404Error(string filename)
{
    if (fileStorInode.find(filename) == fileStorInode.end())
    {
        printf("File Not Found\n");
        return -1;
    }
    return 0;
}
int openDeleteError(int i,int currentPointer)
{
    if(FDMap.find(i) != FDMap.end())
    {
        if(FDMap[i].first == currentPointer)
        {
            printf("File is currently open cannot delete\n");
            return -1;
        }
    }
    return 0;
}
int fileDeleteMode(char *name)
{
    string filename = string(name);
    if(file404Error(filename)==-1)
    {
        return -1;
    }
    int currentPointer = fileStorInode[filename];
    for (int i = 0; i < NOFD; i++)
    {
        if(openDeleteError(i,currentPointer)==-1)
        {
            return -1;
        }
    }
    InodeFileStorMap.erase(fileStorInode[filename]);
    fileStorInode.erase(filename);
    deleteIContent(currentPointer);
    char emptyname[30] = "";
    inodeFreeV.push_back(currentPointer);
    strcpy(inodeMapArray[currentPointer].fileNameS, emptyname);
    inodeMapArray[currentPointer].inodeNumber = -1;
    printf("File Deleted Sucessfully\n");
    return 0;
}
int fdNotAva()
{
    if (FDVector.size() == 0)
    {
        printf("FD Error\n");
        return -1;
    }
    return 0;
}
int alreadyUsedInFD(int currentPointer,int i)
{
    if(FDMap.find(i) != FDMap.end())
    {
        if(FDMap[i].first == currentPointer)
        {
            if((FDMMap[i] == 1 || FDMMap[i] == 2))
            {
                return -1;
            }
        }
    }
    return 0;
}
int fileOpenMode(char *name)
{
    string filename = string(name);
    int RWAMode = -1;
    
    if(file404Error(filename)==-1)
    {
        return -1;
    }
    if(fdNotAva()==-1)
    {
        return -1;
    }
    int currentPointer = fileStorInode[filename];
    while(true)
    {
        printf("0. Read Mode\n1. Write Mode\n2. Append Mode\n");
        cin>>RWAMode;
        if(RWAMode>=0&&RWAMode<=2)
        {
            break;
        }
        printf("Please Enter Valid Choice\n");
    }
    if (RWAMode == 1 || RWAMode == 2)
    {
        for (int i = 0; i < NOFD; i++)
        {
            if(alreadyUsedInFD(currentPointer,i)==-1)
            {
                printf("File is in Use\n");
                return -1;
            }
        }
    }

    int fd = FDVector.back();
    FDMap[fd].first = currentPointer;
    FDMap[fd].second = 0;
    FDVector.pop_back();
    fileCount=fileCount+1;
    FDMMap[fd] = RWAMode;

    string str="File: "+filename+", FD: "+to_string(fd);
    cout<<str<<endl;

    return fd;
}


int fileCloseMode(int fd)
{
    if (FDMap.find(fd) != FDMap.end())
    {
        FDMap.erase(fd);
        fileCount=fileCount-1;
        FDMMap.erase(fd);
        FDVector.push_back(fd);
        printf("File Closed\n");
        return 1;
    }
    else
    {
        printf("Please Open file First\n");
        return -1;
    }
}
string terminalData()
{
    printf("Enter Data\n");
    cout.flush();
    string temp="",tempStr;
    getline(cin, tempStr);
    temp += (tempStr + "\n");
    while(tempStr!="$")
    {
        getline(cin, tempStr);
        temp += (tempStr + "\n");
    }
    return temp;
}
int *initWriteFile(int fd)
{
    int *result=(int *)malloc(sizeof(int)*3);
    result[0] = FDMap[fd].second;
    result[2] = FDMap[fd].first;
    result[1] = result[0] / DBlock;
    return result;
}
void dataBlockFilled(char *bufferf,int len,int currentNodePosition,int fileNodeSpace,int *sizeInMemory,int fd,int writeInDB)
{
    BWrite(writeInDB, bufferf, len, currentNodePosition % DBlock);
    inodeArryList[fileNodeSpace].fileSize =inodeArryList[fileNodeSpace].fileSize+ len;
    FDMap[fd].second =FDMap[fd].second+ len;
    *sizeInMemory =*sizeInMemory+ len;
}
int *dataBlockFilled1(int fd,int len,char *readFromBuffer)
{
    int *indirectBlockPTR=new int[1024];
    int block = inodeArryList[FDMap[fd].first].pointer[len];
    BRead(block, readFromBuffer);
    memcpy(indirectBlockPTR, readFromBuffer, sizeof(char)*DBlock);
    return indirectBlockPTR;
}
int spaceError()
{
    if (fDBVector.size() == 0)
    {
        printf("No more Space available\n");
        return -1;
    }
    return 0;
}
void dataBlockFilled3(int fileNodeSpace,char *bufferf,int len,int *sizeInMemory,int fd)
{
    inodeArryList[fileNodeSpace].fileSize =inodeArryList[fileNodeSpace].fileSize + len;
    FDMap[fd].second =FDMap[fd].second + len;
    *sizeInMemory = *sizeInMemory+ len;
}
void dataBlockFilled2(int fileNodeSpace,char *bufferf,int len,int *sizeInMemory,int fd)
{
    BWrite(inodeArryList[fileNodeSpace].pointer[0], bufferf, len, 0);
    dataBlockFilled3(fileNodeSpace,bufferf,len,sizeInMemory,fd);
}
int *dataBlockFilled4()
{
    int *indirectBlockPTR=new int[1024];
    for (int i = 0; i < 1024; ++i)
    {
        indirectBlockPTR[i] = -1;
    }
    return indirectBlockPTR;
}
int writeInFileDB1(int fd, char *bufferf, int len, int *sizeInMemory)
{
    
    int *resultTemp=initWriteFile(fd);
    int currentNodePosition = resultTemp[0];
    int DBfilled = resultTemp[1];
    int fileNodeSpace = resultTemp[2];
    int count1=0;
    if (currentNodePosition % DBlock != 0)
    {
        if (DBfilled < 10)
        {
            count1++;
            int writeInDB = inodeArryList[fileNodeSpace].pointer[DBfilled];
            dataBlockFilled(bufferf,len,currentNodePosition,fileNodeSpace,sizeInMemory,fd,writeInDB);
        }
        else if (DBfilled < 1034)
        {
            char readFromBuffer[DBlock];
            int *indirectBlockPTR=dataBlockFilled1(fd,10,readFromBuffer);
            count1++;
            int writeInDB = indirectBlockPTR[DBfilled - 10];
            dataBlockFilled(bufferf,len,currentNodePosition,fileNodeSpace,sizeInMemory,fd,writeInDB);
        }
        else
        {
            char readFromBuffer[DBlock];
            int *indirectBlockPTR=dataBlockFilled1(fd,11,readFromBuffer);
            int indirectBlockPTR2[1024]; 
            int block2 = indirectBlockPTR[(DBfilled - 1034) / 1024];
            BRead(block2, readFromBuffer); 
            memcpy(indirectBlockPTR2, readFromBuffer, sizeof(readFromBuffer));
            count1++;
            int writeInDB = indirectBlockPTR[(DBfilled - 1034) / 1024];
            dataBlockFilled(bufferf,len,currentNodePosition,fileNodeSpace,sizeInMemory,fd,writeInDB);
        }
    }
    else
    {
        if (inodeArryList[FDMap[fd].first].fileSize == 0)
        {
            dataBlockFilled2(fileNodeSpace,bufferf,len,sizeInMemory,fd);
        }
        else
        {
            if (currentNodePosition == 0)
            {
                if (FDMap[fd].second == 0)
                {
                    deleteIContent(fileNodeSpace);
                    if(spaceError()==-1)
                    {
                        return -1;
                    }
                    int nAvailnodeDB = fDBVector.back();
                    inodeArryList[fileNodeSpace].pointer[0] = nAvailnodeDB;
                    fDBVector.pop_back();
                }
                dataBlockFilled2(fileNodeSpace,bufferf,len,sizeInMemory,fd);
            }
            else
            {
                if (DBfilled < 10)
                {
                    if(spaceError()==-1)
                    {
                        return -1;
                    }
                    int writeInDB = fDBVector.back();
                    inodeArryList[fileNodeSpace].pointer[DBfilled] = writeInDB;
                    fDBVector.pop_back();
                    BWrite(writeInDB, bufferf, len, currentNodePosition % DBlock);
                    dataBlockFilled3(fileNodeSpace,bufferf,len,sizeInMemory,fd);
                }
                else if (DBfilled < 1034)
                {
                    count1++;
                    if (DBfilled == 10)
                    {

                        int *indirectBlockPTR=dataBlockFilled4();
                        if(spaceError()==-1)
                        {
                            return -1;
                        }
                        int DBSingleIndirect = fDBVector.back();
                        inodeArryList[fileNodeSpace].pointer[10] = DBSingleIndirect;
                        char tempBuffer[DBlock];
                        fDBVector.pop_back();
                        memcpy(tempBuffer, indirectBlockPTR, DBlock);
                        BWrite(DBSingleIndirect, tempBuffer, DBlock, 0);
                    }
                    count1++;
                    int block = inodeArryList[fileNodeSpace].pointer[10];
                    int indirectBlockPTR[1024];
                    char readFromBuffer[DBlock];
                    count1++;
                    BRead(block, readFromBuffer);                        
                    memcpy(indirectBlockPTR, readFromBuffer, sizeof(readFromBuffer));
                    if(spaceError()==-1)
                    {
                        return -1;
                    }
                    int writeInDB = fDBVector.back();
                    indirectBlockPTR[DBfilled - 10] = writeInDB;
                    char tempBuffer[DBlock];
                    fDBVector.pop_back();
                    memcpy(tempBuffer, indirectBlockPTR, DBlock);
                    BWrite(block, tempBuffer, DBlock, 0);
                    BWrite(writeInDB, bufferf, len, 0);
                    dataBlockFilled3(fileNodeSpace,bufferf,len,sizeInMemory,fd);
                }
            }
        }
    }
    return 0;
}
int writeFileErrorFd(int fd)
{
    if (FDMap.find(fd) == FDMap.end())
    {
        printf("No file with given FD\n");
        return -1;
    }
    return 0;
}
int writeFileErrorW(int fd)
{
    if (FDMMap[fd] != 1)
    {
        printf("Not opened in Write Mode\n");
        return -1;
    }
    return 0;
}
int writeFileErrorA(int fd)
{
    if (FDMMap[fd] != 2)
    {
        printf("Not opened in Append Mode\n");
        return -1;
    }
    return 0;
}
void writeInit(int currentPointer)
{
    for (int i = 0; i < NOFD; i++)
    {
        if(FDMap.find(i) != FDMap.end())
        {
            if(FDMap[i].first == currentPointer)
            {
                if(FDMMap[i] == 0)
                {
                    FDMap[i].second = 0;
                }
            }
        }
    }
}
void writeCommon(int len,string x,int *sizeInMemory,int fd)
{
    char bufferf[len + 1];
    memset(bufferf, 0, len);
    int count1=0;
    memcpy(bufferf, x.c_str(), len);
    bufferf[len] = '\0';
    count1++;
    writeInFileDB1(fd, bufferf, len, sizeInMemory);
}
int writeInFileDB(int fd, int mode)
{
    if(writeFileErrorFd(fd)==-1)
    {
        return -1;
    }
    int currentPointer = FDMap[fd].first;
    if (mode == 1)
    {
        if(writeFileErrorW(fd)==-1)
        {
            return -1;
        }
        writeInit(currentPointer);
    }
    else
    {
        if (writeFileErrorA(fd)==-1)
        {
            return -1;
        }
        FDMap[fd].second = inodeArryList[currentPointer].fileSize;
    }
    int sizeInMemory = 0;
    unsigned int leftDBSize = DBlock - ((FDMap[fd].second) % DBlock);
    string x = terminalData();
    x.pop_back(),x.pop_back();
    cin.clear();
    x = x.substr(1);
    int len = -1;
    if (leftDBSize >= x.size())
    {
        len = x.size();
        writeCommon(len,x,&sizeInMemory,fd);
    }
    else
    {
        len = leftDBSize;
        writeCommon(len,x,&sizeInMemory,fd);
        x = x.substr(len);
        int leftDB = x.size() / DBlock;
        while (leftDB--)
        {
            len = DBlock;
            writeCommon(len,x,&sizeInMemory,fd);
            x = x.substr(len);
        }
        len =  x.size() % DBlock;
        writeCommon(len,x,&sizeInMemory,fd);
    }
    printf("Write in File Success\n");
    return 0;
}
int readError(int fd)
{
    if (FDMap.find(fd) == FDMap.end())
    {
        printf("Please open file First\n");
        return -1;
    }
    return 0;
}
int readModeError(int fd)
{
    if (FDMMap[fd] != 0)
    {
        printf("File Not opend in Read mode\n");
        return -1;
    }
    return 0;
}
int fileReadMode(int fd)
{
    if(readError(fd)==-1)
    {
        return -1;
    }
    if(readModeError(fd)==-1)
    {
        return -1;
    }
    int fs = FDMap[fd].second;
    int readTillNow = 0;
    int currentPointer = FDMap[fd].first;
    struct inode in = inodeArryList[currentPointer];
    int fileSize = in.fileSize;
    char readFromBuffer[DBlock];
    bool halfRead = false;
    char *buffer= new char[fileSize];
    int noOfBlocks = ceil(((float)inodeArryList[currentPointer].fileSize) / DBlock);
    char *IndexBuffer = buffer;
    int tNOBlock = noOfBlocks; 
    
    int i=0;
    int count1=0;
    while(i<10)
    {
        if (noOfBlocks == 0)
        {
            break;
        }
        int blockNumber = in.pointer[i];
        count1++;
        BRead(blockNumber, readFromBuffer);
        if ((tNOBlock - noOfBlocks >= fs / DBlock) && (noOfBlocks > 1))
        {
            if (!halfRead)
            {
                int temp=(DBlock - fs % DBlock);
                memcpy(buffer, readFromBuffer + (fs % DBlock),temp);
                buffer = buffer + temp;
                halfRead = true;
                readTillNow = readTillNow+temp;
            }
            else
            {

                memcpy(buffer, readFromBuffer, DBlock);
                buffer = buffer + DBlock;
                readTillNow =readTillNow+ DBlock;
            }
        }
        noOfBlocks--;
        i++;
    }
    int temp=tNOBlock - fs / DBlock;
    if (temp>1)
    {
        memcpy(buffer, readFromBuffer, (inodeArryList[currentPointer].fileSize) % DBlock);
        readTillNow += (inodeArryList[currentPointer].fileSize) % DBlock;
        count1++;
    }
    else if (temp==1)
    {
        int temp1=fs % DBlock;
        memcpy(buffer, readFromBuffer + (temp1), (inodeArryList[currentPointer].fileSize) % DBlock - temp1);
        readTillNow += (inodeArryList[currentPointer].fileSize) % DBlock - temp1;
    }
    IndexBuffer[readTillNow] = '\0';
    cout.flush();
    printf("%s\n",IndexBuffer);
    cout.flush();
    printf("Read Success\n");
    return 1;
}
void commonUnmount(char *temp,int len)
{
    fwrite(temp, sizeof(char), len, diskPointer);
}
void sbUnmount(char *blockInfoBuffer,int len)
{
    memset(blockInfoBuffer, 0, len);
    memcpy(blockInfoBuffer, &sb, sizeof(sb));
    commonUnmount(blockInfoBuffer,sizeof(sb));
    fseek(diskPointer, (sb.blockInformationSize) * DBlock, SEEK_SET);
}
void dirbufferfUnmount(char *DBBuffer,int len)
{
    memset(DBBuffer, 0, len);
    memcpy(DBBuffer, inodeMapArray, len);
    commonUnmount(DBBuffer,len);
    fseek(diskPointer, (sb.inodeIndexStart) * DBlock, SEEK_SET);
}
void inodebufferfUnmount(char *inodeBufferRead,int len)
{
    memset(inodeBufferRead, 0, len);
    memcpy(inodeBufferRead, inodeArryList, len);
    fwrite(inodeBufferRead, sizeof(char), len, diskPointer);
}
int createDiskCheck(char *vDiskName)
{
    if(access( vDiskName, F_OK ) != -1)
    {
        printf("Disk already Exit with same name please try with another name\n");
        return -1;
    }
    return 0;
}
void pointerInit()
{
    int i,j;
    for (i = 0; i < NOI; ++i)
    {
        for (j = 0; j < 12; j++)
        {
            inodeArryList[i].pointer[j] = -1;
        }
    }
}
void createDiskbufferfer(char *bufferfer)
{
    for (int i = 0; i < DB; ++i)
    {
        fwrite(bufferfer, 1, DBlock, diskPointer);
    }
}
int diskInMemorey(char *vDiskName)
{
    char bufferfer[DBlock];
    if(createDiskCheck(vDiskName)==-1)
    {
        return -1;
    }
    diskPointer = fopen(vDiskName, "wb");
    memset(bufferfer, 0, DBlock);
    struct blockInformation sb;
    int i;
    for (i = 0; i < sb.indexDB; ++i)
    {
        sb.freeListDB[i] = true;
    }
    for (i = sb.indexDB; i < DB; ++i)
    {
        sb.freeListDB[i] = false; 
    }
    for (i = 0; i < NOI; ++i)
    {
        sb.InodeListBool[i] = false;
    }
    pointerInit();
    int len;
    fseek(diskPointer, 0, SEEK_SET);
    len = sizeof(struct blockInformation);
    char blockInfoBuffer[len];
    sbUnmount(blockInfoBuffer,len);
    len = sizeof(inodeMapArray);
    char DBBuffer[len];
    dirbufferfUnmount(DBBuffer,len);
    len = sizeof(inodeArryList);
    char inodeBufferRead[len];
    inodebufferfUnmount(inodeBufferRead,len);
    fclose(diskPointer);
    printf("Disk Created Sucessfully\n");
    return 1;
}
int diskPointerError()
{
    if (diskPointer == NULL)
    {
        printf("Disk not Found Please try with valid disk name\n");
        return 0;
    }
    return 1;
}
void commonMount(char *temp,int len)
{
    fread(temp, sizeof(char), len, diskPointer);
}
void sbbufferfMount(char *blockInfoBuffer)
{
    memset(blockInfoBuffer, 0, sizeof(sb));
    commonMount(blockInfoBuffer,sizeof(sb));
    memcpy(&sb, blockInfoBuffer, sizeof(sb));
    fseek(diskPointer, (sb.blockInformationSize) * DBlock, SEEK_SET);
}
void dirbufferfMount(char *DBBuffer,int len)
{
    memset(DBBuffer, 0, len);
    commonMount(DBBuffer,len);
    memcpy(inodeMapArray, DBBuffer, len);
    fseek(diskPointer, (sb.inodeIndexStart) * DBlock, SEEK_SET);
}
void inodebufferfMount(char *inodeBufferRead,int len)
{
    memset(inodeBufferRead, 0, len);
    fread(inodeBufferRead, sizeof(char), len, diskPointer);
    memcpy(inodeArryList, inodeBufferRead, len);
}
void freeListMount()
{
    int i;
    for (i = NOI - 1; i >= 0; --i)
    {
        if (sb.InodeListBool[i])
        {
            fileStorInode[string(inodeMapArray[i].fileNameS)] = inodeMapArray[i].inodeNumber;
            active=1;
            InodeFileStorMap[inodeMapArray[i].inodeNumber] = string(inodeMapArray[i].fileNameS);
        }
        else
        {
            inodeFreeV.push_back(i);
        }
    }
}
int diskEnter(char *name)
{
    diskPointer = fopen(vDiskName, "rb+");
    if(diskPointerError()==0)
    {
        return 0;
    }
    char blockInfoBuffer[sizeof(sb)];
    sbbufferfMount(blockInfoBuffer);
    int len = sizeof(inodeMapArray);
    char DBBuffer[len];
    dirbufferfMount(DBBuffer,len);
    len = sizeof(inodeArryList);
    char inodeBufferRead[len];
    inodebufferfMount(inodeBufferRead,len);
    freeListMount();
    int i;
    for (i = DB - 1; i >= sb.indexDB; --i)
    {
        if (sb.freeListDB[i] == false)
        {
            fDBVector.push_back(i);
        }
    }
    active = 1;
    for (i = NOFD - 1; i >= 0; i--)
    {
        FDVector.push_back(i);
    }
    printf("Disk Mounted Successfully\n");
    active=1;
    return 1;
}

void clearAll()
{
    fDBVector.clear();
    inodeFreeV.clear();
    fDBVector.clear();
    FDVector.clear();
    FDMMap.clear();
    FDMap.clear();
    fileStorInode.clear();
    InodeFileStorMap.clear();
}
void freeListClear()
{
    int i;
    for (i = DB - 1; i >= sb.indexDB; --i)
    {
        sb.freeListDB[i] = true;
    }
    return;
}
void freeListVectorClear()
{
    unsigned int i=0;
    for ( i = 0; i < fDBVector.size(); i++)
    {
        sb.freeListDB[fDBVector[i]] = false;
    }
    return;
}
void inodeFreeListClear()
{
    int i;
    for (i = 0; i < NOI; ++i)
    {
        sb.InodeListBool[i] = true;
    }
    return;
}
void freeInodeVectorClear()
{
    unsigned i;
    for (i = 0; i < inodeFreeV.size(); ++i)
    {
        sb.InodeListBool[inodeFreeV[i]] = false;
    }
    return;
}
int diskExit()
{
    if(fprintfErrorRead()==-1)
    {
        return -1;
    }
    freeListClear();
    freeListVectorClear();
    inodeFreeListClear();
    freeInodeVectorClear();
    fseek(diskPointer, 0, SEEK_SET);
    printf("Disk unmounted Sucessfully\n");
    
    int len = sizeof(struct blockInformation);
    char blockInfoBuffer[len];
    sbUnmount(blockInfoBuffer,len);
    len = sizeof(inodeMapArray);
    char DBBuffer[len];
    dirbufferfUnmount(DBBuffer,len);
    len = sizeof(inodeArryList);
    char inodeBufferRead[len];
    inodebufferfUnmount(inodeBufferRead,len);
    clearAll();
    fclose(diskPointer);
    active = 0;
    return 0;
}

void option()
{
    int choice;
    int fd = -1;
    while (1)
    {
        printf("1. Create file\n2. Open file\n3. Read file\n4. Write file\n5. Append file\n6. Close file\n7. Delete file\n8. List of files\n9. List of opened files\n10. Unmount\n");
        cin.clear();
        scanf("%d",&choice);
        if(choice==1)
        {
            printf("Enter Filename\t");
            scanf("%s",filename);
            fileCreateInMem(filename);
        }
        else if(choice==2)
        {
            printf("Enter Filename\t");
            scanf("%s",filename);
            fileOpenMode(filename);
        }
        else if(choice==3)
        {
            printf("Enter FD for Read\t");
            scanf("%d",&fd);
            fileReadMode(fd);
            cin.clear();
            cout.flush();
        }
        else if(choice==4)
        {
            printf("Enter FD for Write\t");
            scanf("%d",&fd);
            writeInFileDB(fd, 1);
            cin.clear();
            cout.flush();
        }
        else if(choice==5)
        {
            printf("Enter FD for Append\t");
            scanf("%d",&fd);
            writeInFileDB(fd, 2);
            cin.clear();
            cout.flush();
        }
        else if(choice==6)
        {
            printf("Enter FD for Close file\t");
            scanf("%d",&fd);
            fileCloseMode(fd);
        }
        else if(choice==7)
        {
            printf("Enter filename for delete\t");
            scanf("%s",filename);
            fileDeleteMode(filename);
        }
        else if(choice==8)
        {
            if(fileStorInode.size()==0)
            {
                printf("No File currently in Memory\n");
            }
            else
            {
                printf("List of files\n");
                map<string,int>:: iterator itr;
                for(itr=fileStorInode.begin();itr!=fileStorInode.end();itr++)
                {
                    cout<<itr->first<<endl;
                }
            }
        }
        else if(choice==9)
        {
            if(FDMap.size()==0)
            {
                printf("No Open File\n");
            }
            else
            {
                printf("List of opened files\n");
                map<int,pair<int,int> >:: iterator itr;
                for(itr=FDMap.begin();itr!=FDMap.end();itr++)
                {
                    int fd = itr->first;
                    int inode = (itr->second).first;
                    string temp="File Name: "+(string)(InodeFileStorMap[inode])+",  Descriptor: "+to_string(fd)+", ";
                    if (FDMMap[fd] == 0)
                        temp+="Read mode";
                    if (FDMMap[fd] == 1)
                        temp+="Write mode";
                    if (FDMMap[fd] == 2)
                        temp+="Append mode";
                    cout<<temp<<endl;
                }
            }
        }
        else if(choice==10)
        {
            diskExit();
            return ;
        }
        else
        {
            printf("Wrong Choice Please try again\n");
        }
    }
}

int main()
{
    int choice;
    for(;;)
    {
        printf("1. Create disk\n2. Mount disk\n3. Quit\n");
        cin.clear();
        scanf("%d",&choice);
		if(choice==1)
		{
			cout << "Enter Unique Diskname\t" << endl;
            cin >> vDiskName;
            diskInMemorey(vDiskName);
		}
        else if(choice==2)
        {
            cout << "Enter valid diskname\t" << endl;
            cin >> vDiskName;
            if (diskEnter(vDiskName))
            {
                option();
            }
        }
        else if(choice==3)
        {
            break;
        }
        else
        {
            printf("Wrong Choice Please try Again\n");
        }
   }
    return 0;
}