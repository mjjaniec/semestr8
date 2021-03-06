#include "pomiar_czasu.h"
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main( int argc, char** argv ){ 

	int rank, ranksent, size, source, dest, tag, i, j,k; 
	MPI_Status status;
	double time1,time2;
	int sendAmount = 10000;
	int baseSize = 25;
	
	struct worker { 
		char name[3];
		int age;
		double salary;		
		char department;
		char professionId;
	} base1[baseSize], base2[baseSize];

	MPI_Datatype workerType1, workerType2;
	
	
	
	int blockSize[5] = {10, 1, 1, 1, 1 };
	MPI_Datatype types[5] = { MPI_CHAR, MPI_INT, MPI_DOUBLE, MPI_CHAR, MPI_CHAR };
	MPI_Aint  lb, range, base, offsets[5], tmp;
	
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank ); 
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	char *buf = malloc(baseSize*sizeof(struct worker));
	MPI_Buffer_attach( buf, baseSize*sizeof(struct worker)); 	
	MPI_Get_address( &base1[0].name[0], &offsets[0] ); 
	MPI_Get_address( &base1[0].salary, &offsets[1] ); 
	MPI_Get_address( &base1[0].age, &offsets[2] );
	MPI_Get_address( &base1[0].department, &offsets[3] );
	MPI_Get_address( &base1[0].professionId, &offsets[4] );
	
	if( rank == 0 ){
		printf("address name \t %ld\n", offsets[0]);
		printf("address salary \t %ld\n", offsets[1]);
		printf("address age \t %ld\n", offsets[2]);
		printf("address department \t %ld\n", offsets[3]);
		printf("address professionId \t %ld\n", offsets[4]);
		MPI_Get_address( &base1[1].name[0], &tmp ); 
		printf("address id2 \t %ld\n", tmp);
	}

	base =  offsets[0] ;
	for( i=0; i<5; i++ ) 
		offsets[i] -= base ;

	MPI_Type_struct( 5, blockSize, offsets, types, &workerType1 );
	MPI_Type_get_extent( workerType1, &lb, &range );

	if( rank == 0 ){
		printf("workerType1 - lb = %ld, range = %ld\n", lb, range);
	}

	MPI_Type_create_resized( workerType1, lb, range, &workerType2);  

	MPI_Type_commit(&workerType2); 

	if( rank != 0 ){ 
		dest=0; tag=0; 
		for(k=0;k<10;k++)
		{
			base1[10*(rank-1)+k].name[0] = 'B';
			base1[10*(rank-1)+k].name[1] = 'O';
			base1[10*(rank-1)+k].name[2] = 'B';

			base1[10*(rank-1)+k].salary = 307.574;
			base1[10*(rank-1)+k].age = 58;

			base1[10*(rank-1)+k].department = 'P';
			base1[10*(rank-1)+k].professionId = 'D';
		}

		printf("seding: %c %c %c, %lf,  %d,  %c, %c \n", base1[10*(rank-1)].name[0], base1[10*(rank-1)].name[1],base1[10*(rank-1)].name[2],
		base1[10*(rank-1)].salary, base1[10*(rank-1)].age,base1[10*(rank-1)].department,base1[10*(rank-1)].professionId);
	} 
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank == 0)
	{
		inicjuj_czas();
		time1 = czas_zegara();
	}
	MPI_Barrier(MPI_COMM_WORLD);

	if(rank!=0)
	{
		for(k=0;k<sendAmount;k++)
		MPI_Bsend( &base1[10*(rank-1)],10, workerType2, dest, tag, MPI_COMM_WORLD );
	} else
	{
		for( i=1; i<size; i++ ) { 
			for(k=0;k<sendAmount;k++){
				MPI_Recv( &base1[10*(i-1)],10, workerType2, i, 
				MPI_ANY_TAG, MPI_COMM_WORLD, &status );
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0)
	{
		time2 = czas_zegara();
		printf("\nCzas przesylania = %lf \n",time2-time1);
		printf("\nPrzeslanie typu spakowanego\n");
	}

	MPI_Barrier(MPI_COMM_WORLD);

	int siz, siz_package, messageSize, position; void* bufor; 
	MPI_Pack_size(3, MPI_CHAR, MPI_COMM_WORLD, &siz); siz_package = siz;
	MPI_Pack_size(1, MPI_INT, MPI_COMM_WORLD, &siz); siz_package += siz;
	MPI_Pack_size(1, MPI_DOUBLE, MPI_COMM_WORLD, &siz); siz_package += siz;	
	MPI_Pack_size(1, MPI_CHAR, MPI_COMM_WORLD, &siz); siz_package += siz;
	MPI_Pack_size(1, MPI_CHAR, MPI_COMM_WORLD, &siz); siz_package += siz;
	messageSize = 10*siz_package;  
	bufor = (void *)malloc(messageSize);

	if( rank != 0 ){
		dest=0; tag=0; 
		position = 0;
		for( i= 10*(rank-1);i<10*(rank-1)+10;i++ ) {
			MPI_Pack(&base1[i].name[0], 3, MPI_CHAR, 
			bufor, messageSize, &position, MPI_COMM_WORLD );
			MPI_Pack(&base1[i].age, 1, MPI_INT, 
			bufor, messageSize, &position, MPI_COMM_WORLD );
			MPI_Pack(&base1[i].salary, 1, MPI_DOUBLE, 
			bufor, messageSize, &position, MPI_COMM_WORLD );		
			MPI_Pack(&base1[i].department, 1, MPI_CHAR, 
			bufor, messageSize, &position, MPI_COMM_WORLD );
			MPI_Pack(&base1[i].professionId, 1, MPI_CHAR, 
			bufor, messageSize, &position, MPI_COMM_WORLD );
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	if(rank == 0)
		time1 = czas_zegara();
	 
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank!=0){
		for(k=0;k<sendAmount;k++)
			MPI_Bsend( bufor, position, MPI_PACKED, dest, tag, MPI_COMM_WORLD );
	}
	else {
		for( j=1; j<size; j++ ) { 
			for(k=0;k<sendAmount;k++){
				MPI_Recv( bufor, messageSize, MPI_PACKED, j, 
				MPI_ANY_TAG, MPI_COMM_WORLD, &status );

				position = 0;
				for( i=10*(j-1); i<10*(j-1)+10; i++ ) {
					MPI_Unpack(bufor, messageSize, &position, 
					 &base2[i].name[0], 3, MPI_CHAR, MPI_COMM_WORLD );
					MPI_Unpack(bufor, messageSize, &position, 
					 &base2[i].age, 1, MPI_INT, MPI_COMM_WORLD );
					MPI_Unpack(bufor, messageSize, &position, 
					 &base2[i].salary, 1, MPI_DOUBLE, MPI_COMM_WORLD );
					MPI_Unpack(bufor, messageSize, &position, 
					 &base2[i].department, 1, MPI_CHAR, MPI_COMM_WORLD );
					MPI_Unpack(bufor, messageSize, &position, 
					 &base2[i].professionId, 1, MPI_CHAR, MPI_COMM_WORLD );
				}
			}
		}
	}
	MPI_Buffer_detach( &buf, &baseSize ); 
	MPI_Barrier(MPI_COMM_WORLD);
					if(rank == 0)
				{
					time2 = czas_zegara();
					printf("\ntime1 przesylania dla pack = %lf \n",time2-time1);
				}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return(0);
}

