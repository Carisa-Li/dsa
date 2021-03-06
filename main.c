//ref:  https:ww.geeksforgeeks.org/union-and-intersection-of-two-sorted-arrays-2/ (感覺類似merge sort) 
#include "api.h"
#include <stdio.h>
#include<stdlib.h>
#include<string.h>

int n_mails, n_queries;
mail *mails;
query *queries;

int i,j,k;  //dev-c++ can not declare i,j in for loop
unsigned long long int hash_max;
int length; //used in for loop
int ans_sum;
int ans[10000];

struct people{
	char name[32];
	int query_id;
	int size;
	unsigned long long int hash;
	int mail;
	int type; //0 is to,1 is from
	struct people *head;
};
struct people person[20000]; //n_mail<=10000
struct people *To[10000],*From[10000];

struct tokens{
	char *word;
	unsigned long long int hash;
};

struct tokens_array{
	struct tokens *token;
	int sum;
	int size; //size of token
	double sim[10000];
	int used;
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

void init_token(int index,int begin,int type,int l,unsigned long long int hash){
	if(set[index].sum+1>=set[index].size){
		set[index].token=(struct tokens *)realloc(set[index].token,5*set[index].size*sizeof(struct tokens));
		set[index].size*=5;
	}
	set[index].token[set[index].sum].word=(char *)malloc((l+1)*sizeof(char));
	if(type==0) strncpy(set[index].token[set[index].sum].word,mails[index].subject+begin,l+1);
	else{strncpy(set[index].token[set[index].sum].word,mails[index].content+begin,l+1);}
	set[index].token[set[index].sum].word[l]='\0';
	set[index].token[set[index].sum].hash=hash;
	set[index].sum++;
}

void pre_processing_1(){
	int l,begin,n_re,cmp;
	unsigned long long int hash;
	for(i=0;i<n_mails;i++){
		set[i].token=(struct tokens *)malloc(100*sizeof(struct tokens));
		set[i].sum=0;
		set[i].size=100;
		set[i].used=0;
		length=strlen(mails[i].subject);
		l=0;
		begin=0;
		hash=0;
		for(j=0;j<=length;j++){
			if(isalnum(mails[i].subject[j])){
				l++;
				hash*=26;
				if(isupper(mails[i].subject[j])) mails[i].subject[j]=tolower(mails[i].subject[j]);
				if(isdigit(mails[i].subject[j])) hash+=mails[i].subject[j]-'0';
				else if(islower(mails[i].subject[j])) hash+=mails[i].subject[j]-'a';
				if(hash>=hash_max) hash%=hash_max;
			}
			else{
				if(l){
					init_token(i,begin,0,l,hash);
					l=0;
					hash=0;
				}
				begin=j+1;
			}
		}
		length=strlen(mails[i].content);
		l=begin=hash=n_re=0;
		for(j=0;j<=length;j++){
			if(isalnum(mails[i].content[j])){
				l++;
				hash*=26;
				if(isupper(mails[i].content[j])) mails[i].content[j]=tolower(mails[i].content[j]);
				if(isdigit(mails[i].content[j])) hash+=mails[i].content[j]-'0';
				else if(islower(mails[i].content[j])) hash+=mails[i].content[j]-'a';
				if(hash>=hash_max) hash%=hash_max;
			}
			else{
				if(l){
					init_token(i,begin,1,l,hash);
					l=0;
					hash=0;
				}
				begin=j+1;
			}
		}
		qsort(set[i].token,set[i].sum,sizeof(struct tokens),cmp_token);
		for(j=1;j<set[i].sum;j++){
			cmp=cmp_token(&set[i].token[j],&set[i].token[j-1]);
			if(cmp==0) n_re+=1;
			else if(n_re>0) set[i].token[j-n_re]=set[i].token[j];
		}
		set[i].sum=set[i].sum-n_re;
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
		init_person(2*i+1,i,1);
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

//subtask 3 begin
struct people *find_set(struct people *p,int id){
	if(p->query_id!=id){
		p->query_id=id;
		p->head=p;
		p->size=1;
		return p;
	}
	if(p->head!=p) p->head=find_set(p->head,id);
	return p->head;
}
//subtask 3 end

//subtask 2 begin
void similar(int index){
	ans_sum=0;
	int ptrA,ptrB,mid,index_ans;
	mid=queries[index].data.find_similar_data.mid;
	double similarity,cmp,A=(double)set[mid].sum,B,intersect,threshold;
	for(j=0;j<n_mails;j++){ 
		if(j==mid) continue;
		if(set[j].used||set[mid].used){
			similarity=set[mid].sim[j];
		}
		else{
			ptrA=ptrB=intersect=0;
			B=(double)set[j].sum;
			while(ptrA<A&&ptrB<B){
				cmp=cmp_token(&set[mid].token[ptrA],&set[j].token[ptrB]);
				if(cmp>0) ptrB+=1;
				else if(cmp<0) ptrA+=1;
				else{
					ptrA+=1;
					ptrB+=1;
					intersect+=1;
				}
			}
			similarity=intersect/(A+B-intersect);
			set[mid].sim[j]=similarity;
			set[j].sim[mid]=similarity;
		}
		if(similarity>queries[index].data.find_similar_data.threshold) ans[ans_sum++]=j;
	}
	set[mid].used=1;
}
//subtask2 ends

int main(void) {
	api.init(&n_mails, &n_queries, &mails, &queries);
	hash_max=709490156681136557; //the max prime<(unsigned_long_long_int_MAX-26)/26
	ans_sum=0;
	struct people *A,*B;
	pre_processing_1();
	pre_processing_2();
	
	for(i=0;i<n_queries;i++){
		if(queries[i].type==group_analyse){  //subtask3 group_analyse
			ans[0]=0;
			ans[1]=0;
			for(j=0;j<queries[i].data.group_analyse_data.len;j++){
				A=To[queries[i].data.group_analyse_data.mids[j]];
				B=From[queries[i].data.group_analyse_data.mids[j]];
				A=find_set(A,queries[i].id);
				B=find_set(B,queries[i].id);
				if(A!=B){
					if(A->size==1&&B->size==1) ans[0]++;
					else if(A->size>1&&B->size>1) ans[0]--;
					if(A->size>=B->size){
						B->head=A;
						A->size+=B->size;
						if(A->size>ans[1]) ans[1]=A->size;
					}
					else{
						A->head=B;
						B->size+=A->size;
						if(B->size>ans[1]) ans[1]=B->size;
					}
				}
			}
			api.answer(queries[i].id,ans,2);
		}
	}
	for(i=0;i<n_queries;i++){
		if(queries[i].type==find_similar){  //subtask2 find_similar
			similar(i); 
			api.answer(queries[i].id, ans, ans_sum);
		}
	} 
	return 0;
}
