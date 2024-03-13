#include<iostream>
#include<stdio.h>
#include<pthread.h>
#include<fstream>
#include<unistd.h>
#include<time.h>
#include<semaphore.h>
#include<stdlib.h>
#include<string.h>
#include <queue>
#include<linux/kernel.h>
#include<sys/syscall.h>
using namespace std;

    int read_mutex;
    int write_mutex;
    int resource_mutex;
   
    int read_count=0;
    int avail_seats=10;
   
struct FlightData {
string name;
string flight_no;
string destinations;
string special_needs;
int flight_count;
string passport_no;

};
struct ReservationData {
queue<FlightData> request;

}priority,temp2,normal;
void  ReadFromFile(FlightData d[5]) {
int i=0;

ifstream inputfile("flight_data.txt");


if (!inputfile) {
 cout<<"Error opening file!"<<endl;
}

else {
//file.ignore(5,'\n');
while(1) {
inputfile>> d[i].name >> d[i].passport_no >> d[i].flight_no >>d[i].destinations>> d[i].special_needs >> d[i].flight_count;        
i++;
if(inputfile.eof())
 break;
}
}
inputfile.close();
}
class RWLocks {  
public:
    void lock_read() {
        syscall(336,read_mutex); //wait
        read_count++;
        if (read_count == 1) {
            syscall(336,resource_mutex); //wait 
        }
        syscall(337,read_mutex); // post
    }

    void unlock_read() {
        syscall(336,read_mutex); //wait
        read_count--;
        if (read_count == 0) {
            syscall(337,resource_mutex);//post
        }
        syscall(337,read_mutex); //post
    }

    void lock_write() {
        syscall(336,write_mutex);//wait
        syscall(336,resource_mutex);//wait
    }

    void unlock_write() {
        syscall(337,resource_mutex);//post
        syscall(337,write_mutex); //post
    }
};
void*bookSeat(void *arg) {
int id=*((int*)arg);
 FlightData d[5];
 RWLocks lockobj;
 int flag=1;
      int i=0;
      int count=0;
lockobj.lock_write();
ReadFromFile(d);
cout<<"\nwriter id "<<id<<endl;
while(i<4) {
/*if (avail_seats <0 ){
cout<<"sorry all seats are reserved";
break;
}*/
if(avail_seats > 0 && (d[i].special_needs=="true") || d[i].flight_count>=5) {
 
//     cout<<"\nEnter your desired destinations for "<<d[i].name<<endl;
// while(count <5 && flag !=0){
//  cin>>d[i].destinations[count];
//  count++;
//  cout<<"Press 1 to add more and press 0 to stop\n";
//  cin>>flag;
// }
 priority.request.push(d[i]);
 avail_seats--;
     
 cout<<"Reserved seat for " <<d[i].name<<" on flight no "<<d[i].flight_no<<" to "<<d[i].destinations<<endl;
 cout<<"Available seats are "<<endl<<avail_seats<<endl;
 //break;
}

else{
count=0;
//cout<<"\nNot a priority client will reserve later!"<<endl;
//   cout<<"\nEnter your desired destinations for "<<d[i].name<<endl;
// while(count <5 && flag !=0){
//  cin>>d[i].destinations[count];
//  count++;
//  cout<<"Press 1 to add more and press 0 to stop\n";
//  cin>>flag;
// }

 normal.request.push(d[i]);
 }
  flag=1;
i++;
}

while(!normal.request.empty()){
priority.request.push(normal.request.front());
cout<<"Reserved seat for " <<normal.request.front().name<<" on flight no "<<normal.request.front().flight_no<<"to"<<normal.request.front().destinations<<endl;
    avail_seats--;
    cout<<"Available seats are "<<endl<<avail_seats<<endl;
   
    normal.request.pop();
}

lockobj.unlock_write();
}
/*void booknorm(){
while(!normal.request.empty()){

cout<<"Reserved seat for " <<normal.request.front().name<<" on flight no "<<normal.request.front().flight_no;

    avail_seats--;
     
    cout<<"Available seats are "<<endl<<avail_seats<<endl;
    priority.request.push(normal.request.front());
    normal.request.pop();
}
}*/
void*cancel_seat(void*arg){
string pas=*((string*)arg);
 RWLocks lockobj;
lockobj.lock_write();

int i=0;
int flag=0;

while(!priority.request.empty()){
if(pas==priority.request.front().passport_no){
cout<<"Cancelled seat for  "<<priority.request.front().name<<" on flight number "<<priority.request.front().flight_no<<endl;
    priority.request.pop();
    avail_seats++;

cout<<"Available seats are "<<endl<<avail_seats<<endl;
flag=1;
break;
}
else{
temp2.request.push(priority.request.front());
    priority.request.pop();
    continue;
}

}

while(!temp2.request.empty()){
priority.request.push(temp2.request.front());
temp2.request.pop();
}
lockobj.unlock_write();
if(flag==0){
cout<<"No passenger found!";
}
}
void *reader(void*arg) {
int id = *((int*)arg);
FlightData d[5];
 RWLocks lockobj;
 lockobj.lock_read();
 ReadFromFile(d);
printf("\nreader id:%d\nreading flight data:\n",id);
cout<<d[id].name<<" "<<d[id].passport_no<<" "<<d[id].flight_no<<" "<<d[id].destinations<<" "<<d[id].special_needs<<endl;
cout<<"Read count is "<<read_count<<endl;
read_count++;
lockobj.unlock_read();
}
int main(){
sem_init(&read_mutex,0,1);
sem_init(&write_mutex,0,1);
sem_init(&resource_mutex,0,1);
read_mutex=1,write_mutex=1,resource_mutex=1;
pthread_t readers[5],reserver[2], cancelc;
int readerid[5]={0,1,2,3,4};
int reserverid[2]={0,1};
string pas;
for(int i=0;i<2;i++){
pthread_create(&reserver[i],NULL,bookSeat,&reserverid[i]);
}
for(int i=0;i<4;i++){
pthread_create(&readers[i],NULL,reader,&readerid[i]);
}

for(int i=0;i<2;i++){
pthread_join(reserver[i],NULL);
}
for(int i=0;i<4;i++){
pthread_join(readers[i],NULL);
}
cout<<"\nenter your passport number to cancel your reservation:";
cin>>pas;
pthread_create(&cancelc,NULL,cancel_seat,&pas);
pthread_join(cancelc,NULL);

//booknorm();
sem_destroy(&read_mutex);
sem_destroy(&write_mutex);
sem_destroy(&resource_mutex);
}
