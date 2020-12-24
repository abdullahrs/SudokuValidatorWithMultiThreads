/*
Abdullah Rauf Simsek
180701005
Isletim Sistemleri Proje Odevi
Sudoku Dogrulayici
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
// 9 thread sutunlari kontrol etmek icin
// 9 thread satirlari kontrol etmek icin
// 9 thread 3x3 alt matrisleri kontrol etmek icin
// Toplamda 27 thread olusacak (+ main thread)
#define thread_count  27
// Tum degerleri 0'a initialize ediyoruz
// Thread'ler calisirken bir tutarsizlik olusursa
// thread'in array'deki degerini 1 yapiyoruz
// bu sayade main thread tarafindan tum diger
// thread'larin kontrol edilmesini sagliyoruz
int validation_array[thread_count] = {0};
// Kullanicidan alinacak degerlerin tutulacagi matrix
int sudoku_matrix[9][9] = {0};
// Test icin sudoku matriks'i
int test_matrix[9][9] = {
        {4,3,5,2,6,9,7,8,1},
        {6,8,2,5,7,1,4,9,3},
        {1,9,7,8,3,4,5,6,2},
        {8,2,6,1,9,5,3,4,7},
        {3,7,4,6,8,2,9,1,5},
        {9,5,1,7,4,3,6,2,8},
        {5,1,9,3,2,6,8,7,4},
        {2,4,8,9,5,7,1,3,6},
        {7,6,3,4,1,8,2,5,9}
    };
/*Thread fonksiyonlarina kontrol edecekleri satir, sutun ve altmatrislerin
bilgilerini gecmek lazim fakat thread fonksiyonlari sadece bir parametre
aliyor bir struct olusturarak birden fazla parametreyi gecebiliyoruz*/
// Thread fonksiyonlarina gonderilecek parametre yapisi satir ve sutun
// bilgilerini iceriyor.
typedef struct details
{
    int row_index;
    int column_index;
}details;
// Kullanicidan sudoku degerlerinin alinacagi fonksiyon
void getValues()
{
    int i = 0,j=0;
    for (i; i < 9; i++)
    {
        for(int j = 0;j<9;j++)
        {
            printf("[%d][%d] boslugundaki elemani giriniz :",i+1,j+1);
            scanf("%d",&sudoku_matrix[i][j]);
        }
    }
}

void *checkColumn(void *param)
{
    // TODO: hoca'nin uygulama dersinde soyledigi bir terimi yaz
    details *dets = (details *) param;
    // pointer deki bilgiler int degerlere ataniyor
    int row_index = dets->row_index;
    int col_index = dets->column_index;
    // Girilen degerlerin dogrulugu kontrol ediliyor
    // Eger row_index degeri 0 degilse tum kolon degerleri alinamaz
    // Eger col_index degeri 0=<col_index=<8 degilse tasma olur
    if(row_index != 0 || col_index < 0 || col_index > 8 )
    {
        printf("Girilen degerler yanlis satir: %d sutun: %d \n",row_index,col_index);
        pthread_exit(NULL);
    }
    // Sutundaki degerlerin takibini yapmak icin gerekli array
    int tempArray[9] = {0};
    int i=0;
    for (i; i < 9; i++)
    {
        // Degerin 1-9 kapali araliginda olup olmadigini kontrol ediyor
        if(sudoku_matrix[i][col_index] < 1 || sudoku_matrix[i][col_index]>9)
        {
            printf("Gecersiz deger degerler [1-9] araliginda olabilir. Degeriniz : %d\n",sudoku_matrix[i][col_index]);
            pthread_exit(NULL);
        }
        // Sutunda bir tekrar olup olmadigini kontrol ediyor
        else if(tempArray[sudoku_matrix[i][col_index]-1] == 1)
        {
            printf("%d'inci sutun uygun degil\n",col_index);
            pthread_exit(NULL);
        }
        // Eger yukardaki durumlara sahip degilse tempArray deki degerini
        // 1 yapiyoruzki bu degerin kullanildigini tutalim
        else
        {
            tempArray[sudoku_matrix[i][col_index] -1 ] = 1;
        }
    }
    // Eger yukardaki exit'lardan etkilenmeden kodun bu kismina gelebildiyse
    // kolondaki degerler uygundur thread degerine 1 koyarak bu kolondaki
    // degerlerin uygun oldugunu tutuyoruz
    validation_array[18+col_index] = 1;
    pthread_exit(NULL);
}

void *checkRow(void *param)
{
    // TODO: hoca'nin uygulama dersinde soyledigi bir terimi yaz
    details *det = (details*) param;
    // pointer deki bilgiler int degerlere ataniyor
    int row_index = det->row_index;
    int col_index = det->column_index;

    if(col_index != 0 || row_index < 0 || row_index > 8)
    {
        printf("Girilen degerler yanlis satir: %d sutun: %d \n",row_index,col_index);
        pthread_exit(NULL);
    }

    int tempArray[9] = {};
    int i = 0;
    for(i;i<9;i++)
    {
        // Degerin 1-9 kapali araliginda olup olmadigini kontrol ediyor
        if(sudoku_matrix[row_index][i] < 1 || sudoku_matrix[row_index][i] > 9)
        {
            printf("Gecersiz deger degerler [1-9] araliginda olabilir. Degeriniz : %d\n",sudoku_matrix[row_index][i]);
            pthread_exit(NULL);
        }
        // Satirda bir tekrar olup olmadigini kontrol ediyor
        else if(tempArray[sudoku_matrix[row_index][i] - 1] == 1)
        {
            printf("%d'inci satir uygun degil\n",row_index);
            pthread_exit(NULL);
        }
        // Eger yukardaki durumlara sahip degilse tempArray deki degerini
        // 1 yapiyoruzki bu degerin kullanildigini tutalim
        else
        {
            tempArray[sudoku_matrix[row_index][i] - 1] = 1;
        }
    }
    validation_array[9 + row_index] = 1;
    pthread_exit(NULL);
}

void *checkSubMatrix(void *param)
{
    // TODO: hoca'nin uygulama dersinde soyledigi bir terimi yaz
    details *det = (details*) param;
    // pointer deki bilgiler int degerlere ataniyor
    int row_index = det->row_index;
    int col_index = det->column_index;
    // Alt matrislerin sol ust kosesindeki indeksleri ile islem yapilacak
    // gelen degerlerin uygun olup olmadigi kontrol ediliyor
    if(row_index < 0 || row_index > 6 || row_index % 3 != 0 || col_index < 0 || col_index > 6 || col_index % 3 != 0)
    {
        printf("Gecersiz satir ya da sutun index degeri! satir: %d sutun: %d",row_index,col_index);
        pthread_exit(NULL);
    }

    int tempArray[9] = {0};
    int i = row_index,j;

    for (i; i < row_index+3; i++)
    {
        for (j = col_index; j < col_index+3; j++)
        {
            if (sudoku_matrix[i][j] < 0 || sudoku_matrix[i][j] > 9)
            {
                printf("Gecersiz deger degerler [1-9] araliginda olabilir. Degeriniz : %d\n",sudoku_matrix[row_index][i]);
                pthread_exit(NULL);
            }
            else if (tempArray[sudoku_matrix[i][j] - 1] == 1)
            {
                printf("%d'inci alt matriks uygun degil\n",col_index+row_index);
                pthread_exit(NULL);
            }   
        }
    }
    

}


int main(int argc, char const *argv[])
{
    getValues();
    return 0;
}
