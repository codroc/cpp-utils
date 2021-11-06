#include <iostream>
#include <vector>

#include "timestamp.h"
using namespace std;

void all_subset( int arr[], unsigned int size, vector<bool>& contains, int depth )
{
    //when reach the needed length, output
    if ( depth == size )
     {
        for( int j = 0 ; j < size ; j++ )
         {
            if ( contains[j] ) {}
                 // cout<<arr[j]<<" " ;
         }
         // cout<< endl;
     }
    else
     {
         // generate the result that doesn't contain arr[depth]
         contains[depth] = false ;
         all_subset( arr, size, contains, depth+1 );
        // generate the result that contains arr[depth]
         contains[depth] = true ;
         all_subset( arr, size, contains, depth+1 );                      
     }
    return ;
}

int main()
{
    int s[] = { 16, 17, 18, 19, 20, 1, 2,
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    int size = sizeof(s)/sizeof(int );
     vector<bool> contains( size, false );
	 Timestamp start = Timestamp::now();
     all_subset( s, size, contains, 0 );
	 Timestamp end = Timestamp::now();
	 long long time = end - start;
	 cout << "finished\nuse " << time << "us\n";
}
