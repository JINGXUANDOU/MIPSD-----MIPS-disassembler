#include "linkedlist.h"
#include "hw2_helpers.h"
#include "hw2.h"

#define LINE_SIZE	255

// Part 1 Functions
int getSubstrings(char *str,  char delim, char ** array, int maxSize){
    if (!str || !array || maxSize <= 0)
        return -1;

    int arr_count = 0;
    char* s = str;

    if (*str == '\0' || delim == '\0')
        return 0;

    while (arr_count < maxSize) {
        *(array + arr_count) = s;
        arr_count++;

        while (*s != delim && *s != '\0')
            s++;

        if (*s == '\0')
            break;

        *s++ = '\0';
    }

    return arr_count;
}

void parseMIPSfields(const uint32_t instruction, MIPSfields* f){
    f->func = (uint8_t)instruction & 0x3f;
    f->shamt = (uint8_t)(instruction >> 6) & 0x1f;
    f->rd = (uint8_t)(instruction >> 11) & 0x1f;
    f->rt = (uint8_t)(instruction >> 16) & 0x1f;
    f->rs = (uint8_t)(instruction >> 21) & 0x1f;
    f->opcode = (uint8_t)(instruction >> 26);

    f->immediate16 = (uint16_t)instruction;
    f->immediate26 = instruction & 0x3ffffff;

    if (f->opcode == 0x0) // R-type
        f->uid = (uint32_t)f->func;
    else  // I-type and J-type
        f->uid = ((uint32_t)f->opcode) << 26;
}

MIPSinstr* loadInstrFormat(char* line) {
    if (line == NULL)   // basic NULL check
        return NULL;

    // Read and get substrings
    char** substrings = malloc(4*sizeof(char *));
    int size = 1;   // 1 is prepared for later '\0'
    char* line_go = line;

    while (*line_go != '\0'){
        ++size;
        ++line_go;
    }

    char* oristring = malloc(size * sizeof(char *));
    char* oristring_write = oristring;

    while (*line != '\0'){
        *oristring_write = *line;
        ++oristring_write;
        ++line;
    }
    *oristring_write = '\0';

    int times = getSubstrings(oristring, ' ', substrings, 4);

    // length check
    if (times != 4){
        free(substrings);
        free(oristring);
        return NULL;
    }

    // type check
    char** substrings_go = substrings;
    char type;

    if ((**substrings_go == 'r' || **substrings_go == 'i' || **substrings_go == 'j') && *(*substrings_go+1) == '\0') {
        type = **substrings_go;
        ++substrings_go;
    }else{
        free(substrings);
        free(oristring);
        return NULL;
    }

    // uid check
    char* uid_go = *substrings_go;
    char* uid = uid_go;
    int uid_digit = 0;

    while (((*uid_go >= '0' && *uid_go <= '9') || (*uid_go >= 'a' && *uid_go <= 'f') || (*uid_go >= 'A' && *uid_go <= 'F')) && uid_digit < 8){ //uid
        ++uid_go;
        ++uid_digit;
    }

    unsigned long int num_uid = 0;
    if (uid_digit == 8 && *uid_go == '\0'){
        num_uid = strtoul(uid, NULL, 16);
    }else{
        free(oristring);
        free(substrings);
        return NULL;
    }
    ++substrings_go;

    // mnemonic check
    char* mne_go = *substrings_go;
    char* mne_size_go = mne_go;
    int mne_size = 1;   // reserve one for '\0' at the end

    while (*mne_size_go != '\0') {
        if(*mne_size_go < 'a' || *mne_size_go > 'z'){ // not lowercase, return
            free(oristring);
            free(substrings);
            return NULL;
        }
        ++mne_size;
        ++mne_size_go;
    }

    char* mnemonic = malloc(mne_size * sizeof(char));
    char* mnemonic_write = mnemonic;
    while (*mne_go != '\0') {
        *mnemonic_write = *mne_go;
        ++mnemonic_write;
        ++mne_go;
    }
    *mnemonic_write = '\0';
    ++substrings_go;

    //pretty check
    unsigned long int num_pretty = 0;
    if (((**substrings_go >= '0' && **substrings_go <= '9') && *(*substrings_go+1) == '\n') || ((**substrings_go == '1' && *(*substrings_go+1) == '0') && *(*substrings_go+2) == '\n')) {
        num_pretty = strtoul(*substrings_go, NULL, 10);
    }else{
        free(substrings);
        free(oristring);
        free(mnemonic);
        return NULL;
    }

    MIPSinstr * data = (MIPSinstr * ) malloc(sizeof(MIPSinstr));
    data->type = type;
    data->uid = (uint32_t) num_uid;
    data->pretty = (uint8_t) num_pretty;
    data->mnemonic = mnemonic;
    data->usagecnt = 0;

    free(oristring);
    free(substrings);
    return data;
}



// Part 2 Functions
int MIPSinstr_uidComparator(const void* s1, const void* s2) {
    const MIPSinstr * ss1 = s1;
    const MIPSinstr * ss2 = s2;
    if ((*ss1).uid < (*ss2).uid)
        return -1;
    else if ((*ss1).uid == (*ss2).uid)
        return 0;
    else if ((*ss1).uid > (*ss2).uid)
        return 1;

}

void MIPSinstr_Printer(void* data, void* fp) {
    MIPSinstr* data1 = data;
    fprintf(fp, "%c\t%u\t%u\t%u\t%s\n", data1->type, data1->uid, data1->pretty, data1->usagecnt, data1->mnemonic);
}

void MIPSinstr_Deleter(void* data) {
    MIPSinstr * data1 = data;
    if (!data1)
        return;

    free(data1->mnemonic);
    free(data1);
    
}

node_t* FindInList(list_t* list, void* token) {
    if (!list || token == NULL)
        return NULL;

    node_t* node = list->head;
    int ptr;
    while (node){
        ptr = list->comparator(node->data, token);

        if (ptr == 0)
            return node;
        else
            node = node->next;
    }
    return NULL;
}

void DestroyList(list_t** list)  {
    node_t* head = (*list)->head;
    node_t* del_node = head;
    while (head){
        MIPSinstr * data1 = head->data;
        free(data1->mnemonic);
        free(data1);
        head = head->next;
        free(del_node);
        del_node = head;
    }
    free(*list);
}


// Part 3 Functions
list_t* createMIPSinstrList(FILE* IMAPFILE) {
    list_t* MIPS_list = CreateList(&MIPSinstr_uidComparator, &MIPSinstr_Printer, &MIPSinstr_Deleter);
    list_t** des_list = &MIPS_list;

	int num_lines = 0;
	char* line_buff;
	
	line_buff = (char*)malloc(LINE_SIZE * sizeof(char));
	//assert(line_buff);

	while (fgets(line_buff, LINE_SIZE, IMAPFILE)) {
        MIPSinstr* data = loadInstrFormat(line_buff);

        if (!data) {
            DestroyList(des_list);
            free(line_buff);
            //free(MIPS_list);
            return NULL;
        }

        node_t* node = MIPS_list->head;
        while (node){
            MIPSinstr* data1 = node->data;
            if (data1->uid == data->uid){
                DestroyList(des_list);
                free(line_buff);
                return NULL;
            }
            node = node->next;
        }

        InsertAtHead(MIPS_list, (void*) data);

    }

    free(line_buff);
    return MIPS_list;
}

int printInstr(MIPSfields* instr, list_t* MIPSinstrList, char** regNames, FILE* OUTFILE)
{
    if (!instr || !MIPSinstrList)
        return 0;

    node_t* node = MIPSinstrList->head;
    while (node){
        MIPSinstr* data1 = node->data;
        if (data1->uid == instr->uid)
            break;
        node = node->next;
    }
    //node_t* node = FindInList(MIPSinstrList, (void*)instr);
    if (node == NULL)
        return 0;

    MIPSinstr* token = (MIPSinstr*)node->data;

    switch (token->pretty) {
    case 0:
        fprintf(OUTFILE, "%s %s\n", token->mnemonic, *(regNames + instr->rd));
        break;
    case 1:
        fprintf(OUTFILE, "%s %s, %s\n", token->mnemonic, *(regNames + instr->rs), *(regNames + instr->rt));
        break;
    case 2:
        fprintf(OUTFILE, "%s %s, %s, 0x%x\n", token->mnemonic, *(regNames + instr->rt), *(regNames + instr->rs), instr->immediate16);
        break;
    case 3:
        fprintf(OUTFILE, "%s %s, %s, %s\n", token->mnemonic, *(regNames + instr->rd), *(regNames + instr->rs), *(regNames + instr->rt));
        break;
    case 4:
        fprintf(OUTFILE, "%s %s, 0x%x(%s)\n", token->mnemonic, *(regNames + instr->rt), instr->immediate16, *(regNames + instr->rs));
        break;
    case 5:
        fprintf(OUTFILE, "%s\n", token->mnemonic);
        break;
    case 6:
        fprintf(OUTFILE, "%s 0x%x\n", token->mnemonic, instr->immediate26);
        break;
    case 7:
        fprintf(OUTFILE, "%s %s, 0x%x\n", token->mnemonic, *(regNames + instr->rs), instr->immediate16);
        break;
    case 8:
        fprintf(OUTFILE, "%s %s, %s, 0x%x\n", token->mnemonic, *(regNames + instr->rd), *(regNames + instr->rs), instr->shamt);
        break;
    case 9:
        fprintf(OUTFILE, "%s %s, %s, 0x%x\n", token->mnemonic, *(regNames + instr->rs), *(regNames + instr->rt), instr->immediate16);
        break;
    case 10:
        fprintf(OUTFILE, "%s %s, 0x%x\n", token->mnemonic, *(regNames + instr->rt), instr->immediate16);
        break;
    default:
        break;
    }
    token->usagecnt++;
    return 1;
}


// Extra Credit Functions
void MIPSinstr_removeZeros(list_t* list) {
    node_t* node = list->head;
    node_t* prev_node = NULL;

    while (node){
        MIPSinstr * data1 = node->data;

        if (data1->usagecnt == 0){
            if (node == list->head)
                list->head = list->head->next;
            else
                prev_node->next = node->next;


            MIPSinstr_Deleter(data1);
            free(node);
        }
        prev_node = node;
        node = node ->next;
    }
}

int MIPSinstr_usagecntComparator(const void* s1, const void* s2) {
    const MIPSinstr* ss1 = s1;
    const MIPSinstr* ss2 = s2;

    if (ss1->usagecnt > ss2->usagecnt)
        return -1;
    else if (ss1->usagecnt > ss2->usagecnt){
        char* ss1_mnemonic = ss1->mnemonic, * ss2_mnemonic = ss2->mnemonic;

        while (*ss1_mnemonic != '\0'){
            if (*ss2_mnemonic == '\0')
                return 1;

            if (*ss1_mnemonic < *ss2_mnemonic)
                return -1;
            else if (*ss1_mnemonic > *ss2_mnemonic)
                return 1;
            
            ++ss1_mnemonic;
            ++ss2_mnemonic;
        }
        return -1;

    }else if (ss1->usagecnt > ss2->usagecnt)
        return 1;
}

void MIPSinstr_statPrinter(void* data, void* fp) {
    MIPSinstr * data1 = data;
    fprintf(fp, "%s\t%d\n", data1->mnemonic, data1->usagecnt);
}

void sortLinkedList(list_t* list) {
    node_t* node = list->head;
    node_t* next_node = node->next;
    for (int i = 1; i < list->length - 1; i++){
        for (int j = 1; j < list->length - i; j++){

        }
    }
}

