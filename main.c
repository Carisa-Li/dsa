#include "api.h"
#include <stdio.h>
#include<stdlib.h>
#include<string.h>

int n_mails, n_queries;
mail *mails;
query *queries;

int i,j;  //dev-c++ can not declare i,j in for loop
int hash_max;
int length; //used in for loop

struct people{
	char name[32];
	int query_id;
	int size;
	int hash;
	int mail;
	int type; //0 is to,1 is from
	struct people *head;
};
struct people person[20000]; //n_mail<=10000
struct people *To[10000],*From[10000];

struct tokens{
	char *word;
	int hash;
};

struct tokens_array{
	struct tokens *token;
	int sum;
	int size; //size of token
};
struct tokens_array set[10000];

//pre-processing 1 begin
int cmp_token(const void *a,const void *b){
	struct tokens *A=(struct tokens *)a;
	struct tokens *B=(struct tokens *)b;
	if(A->hash>B->hash) return 1;
	if(A->hash<B->hash) return -1;
	return strcmp(A->word,B->word);
}

void init_token(int index,int begin,int type,int l,int hash){
	if(set[index].sum+1==set[index].size){
		set[index].token=(struct tokens *)realloc(set[index].token,5*set[index].size*sizeof(struct tokens));
		set[index].size*=5;
	}
	if(type==0) strncpy(set[index].token[set[index].sum].word,mails[index].subject[begin],l+1);
	else{strncpy(set[index].token[set[index].sum].word,mails[index].content[begin],l+1);}
	set[index].token[set[index].sum].word[l]='\0';
	set[index].sum++;
}

void pre_processing_1(){
	int l,begin,hash;
	for(i=0;i<n_mails;i++){
		set[i].token=(struct tokens *)malloc(100*sizeof(struct tokens));
		set[i].sum=0;
		set[i].size=100;
		length=strlen(mails[i].subject);
		l=0;
		begin=0;
		hash=0;
		for(j=0;j<=length;j++){
			if(isalnum(mails[i].subject[j])){
				l++;
				hash*=26;
				if(isupper(mails[i].subject[j])) tolower(mails[i].subject[j]);
				if(isdigit(mails[i].subject[j])) hash+=mails[i].subject[j]-'0';
				if(islower(mails[i].subject[j])) hash+=mails[i].subject[j]-'a';
				if(hash>=hash_max) hash%=hash_max;
			}
			else{
				if(l){
					init_token(i,begin,0,l,hash);
					l=0;
					hash=0;
				}
				begin=i+1;
			}
		}
		length=strlen(mails[i].content);
		l=0;
		begin=0;
		hash=0;
		for(j=0;j<=length;j++){
			if(isalnum(mails[i].content[j])){
				l++;
				hash*=26;
				if(isupper(mails[i].content[j])) tolower(mails[i].content[j]);
				if(isdigit(mails[i].content[j])) hash+=mails[i].content[j]-'0';
				if(islower(mails[i].content[j])) hash+=mails[i].content[j]-'a';
				if(hash>=hash_max) hash%=hash_max;
			}
			else{
				if(l){
					init_token(i,begin,1,l,hash);
					l=0;
					hash=0;
				}
				begin=i+1;
			}
		}
		qsort(set[i].token,set[i].sum,sizeof(struct tokens_array),cmp_token);
	}
}
//pre-processing 1 end

//pre-processing 2 begin
int cmp_name(const void *a,const void *b){
	struct people *A=(struct people *)a;
	struct people *B=(struct people *)b;
	if(A->hash>B->hash) return 1;
	if(A->hash<B->hash) return -1;
	return strcmp(A->name,B->name);
}

void init_person(int index,int mail,int type){
	person[index].head=&person[index];
	person[index].query_id=-1;
	person[index].size=1;
	if(type) strcpy(person[index].name,mails[mail].from);
	else {strcpy(person[index].name,mails[mail].to);}
	person[index].mail=mail;
	person[index].type=type;
	
	person[index].hash=1;
	length=strlen(person[index].name);
	for(j=0;j<length;j++){
		person[index].hash*=26;
		if(isupper(person[index].name[j])) person[index].hash+=person[index].name[j]-'A';
		else if(islower(person[index].name[j])) person[index].hash+=person[index].name[j]-'a';
		else if(isdigit(person[index].name[j])) person[index].hash+=person[index].name[j]-'0';
		if(person[index].hash>=hash_max) person[index].hash%=hash_max;
	}
}

void pre_processing_2(){
	for(i=0;i<n_mails;i++){
		init_person(2*i,i,0);
		init_person(2*i,i,1);
	}
	qsort(person,2*n_mails,sizeof(struct people),cmp_name);
	struct people *now=&person[0];
	for(i=0;i<2*n_mails;i++){
		if(cmp_name(now,&person[i])) now=&person[i];
		if(person[i].type) From[person[i].mail]=now;
		else {To[person[i].mail]=now;}
	}
}
//pre-processing 2 end

int main(void) {
	api.init(&n_mails, &n_queries, &mails, &queries);
	
	hash_max=82595483; //the max prime<(INT_MAX-26)/26
	
	pre_processing_1();
	pre_processing_2();
	int test[]={0};
	for(i=0;i<n_queries;i++){
		if(queries[i].type==expression_match){  //subtask1 expression_match
			api.answer(queries[i].id, NULL, 0);
		}
		else if(queries[i].type==find_similar){  //subtask2 find_similar
			api.answer(queries[i].id, NULL, 0);
		}
		else{  //subtask3 group_analyse
			api.answer(queries[i].id,test , 0);
		}	
	} 
  return 0;
}
