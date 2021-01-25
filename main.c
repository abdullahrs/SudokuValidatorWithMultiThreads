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
// Threadlerin tutuldugu thread arrayi
pthread_t threads[thread_count];
// Threadlerin basarili olup olmadigini tutan global degisken
int validation = 0;
// Kullanicidan alinacak degerlerin tutulacagi matrix
int sudoku_matrix[9][9] = {0};
// Test icin sudoku dogru matris
/*
int sudoku_matrix[9][9] = {
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
*/
// Test icin sudoku yanlis matris
/*
int sudoku_matrix[9][9] = {
{1, 2, 3, 4, 5, 6, 7, 8, 9}, 
{1, 2, 3, 4, 5, 6, 7, 8, 9}, 
{9, 8, 7, 6, 5, 4, 3, 2, 1}, 
{1, 2, 3, 4, 5, 6, 7, 8, 9}, 
{1, 2, 3 ,4, 5, 6, 7, 8, 9}, 
{1, 2, 3, 7, 8, 9, 4, 5, 6}, 
{9, 8, 7, 6, 5, 4, 3, 2, 1}, 
{1, 2, 3, 7, 8, 9, 4, 5 ,6}, 
{1, 2, 3, 4, 5, 6, 7, 8, 9}
    };
*/
/*Thread fonksiyonlarina kontrol edecekleri satir, sutun ve alt matrislerin
bilgilerini gecmek lazim fakat thread fonksiyonlari sadece bir parametre
aliyor bir struct olusturarak birden fazla parametreyi gecebiliyoruz*/
// Thread fonksiyonlarina gonderilecek parametre yapisi satir ve sutun
// bilgilerini iceriyor.
typedef struct details
{
    int row_index;
    int column_index;
}details;
// Thread synchronization icin gerekli mutex lock
pthread_mutex_t lock;
// Compile ederken hata almamak icin gerekli forward declarations
void getValuesOneByOne();
void getValuesRowByRow();
void *checkColumn(void *param);
void *checkRow(void *param);
void *checkSubMatrix(void *param);

int main(int argc, char const *argv[])
{
    // Degerlerin kullanicidan alinacagi fonksiyonun cagrilmasi
    // Tek tek elemanlari almak icin
    getValuesOneByOne();
    // Satir satir elemanlari almak icin
    // getValuesRowByRow();
    // Main thread
    if(pthread_mutex_init(&lock,NULL) != 0)
    {
        printf("Mutex init'i olusturulamadi\n");
        return -1;
    }
    // Thread array'i uzerinde ilerlemek icin isaretci gorevi gorecek
    // int degeri ve dongu degiskenleri
    int thread_index = 0,i = 0,j = 0;

    // 27 adet thread'in olusturulacagi dongu
    for(i;i<9;i++)
    {
        for(j = 0;j < 9 ; j++)
        {
            /*
                Gelen satir ve sutun degerlerine gore gelen deger; satir basi,
                sutun basi veya alt matris basi olabilir bunun icin 3 ayri 
                if bloguyla bakilir.
                if-else if, kullanilmamasinin sebebi bir deger hem satir-sutun
                basi olurken hemde alt matrisin sol ust degeri olmasidir.
            */
            // i degeri 0 ise j'ninci sutun icin bir thread olusturulur
            if(i == 0)
            {
                // Thread de cagiracagimiz fonksiyona gonderecegimiz details
                // parametresi icin allocation yapiyoruz ve gerekli 
                // bilgileri veriyoruz
                details *det = (details *) malloc(sizeof(details));
                det->row_index = i;
                det->column_index = j;
                // Thread olusturuluyor
                pthread_create(&threads[thread_index],NULL,checkColumn,det);
                printf("[%d][checkColumn] Thread olusturuldu.\n",thread_index);
                // Bir sonraki thread'e gecmek icin isaretci bir arttiriliyor
                thread_index++;
            }
            // j degeri 0 ise i'ninci satir icin bir thread olusturulur
            if(j == 0)
            {
                details *det = (details *) malloc(sizeof(details));
                det->row_index = i;
                det->column_index = j;
                pthread_create(&threads[thread_index],NULL,checkRow,det);
                printf("[%d][checkRow] Thread olusturuldu.\n",thread_index);
                thread_index++;
            }
            // i ve j degerinin mod 3 teki degeri 0 ise alt matrisin
            // sol ust kosesindeki deger gelmis demektir icinde bulundugu
            // alt matrisi kontrol etmek icin thread olusturulur
            if(i % 3 == 0 && j % 3 == 0)
            {
                details *det = (details *) malloc(sizeof(details));
                det->row_index = i;
                det->column_index = j;
                pthread_create(&threads[thread_index],NULL,checkSubMatrix,det);
                printf("[%d][checkSubMatrix] Thread olusturuldu.\n",thread_index);
                thread_index++;
            }
        }
    }
    // Main thread'in tum threadleri beklemesi icin tum threadlere join uygulanir
    for ( i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&lock);
    // Tum threadler bittikten sonra valitadion'a bakilir 
    // Eger deger 27 degilse cozum yanlistir
    if (validation != 27)
    {
        printf("Sudoku cozumu yanlis!\n");
        return -1;
    }
    printf("Sudoku cozumu dogru.\n");
    return 0;
}

// Kullanicidan sudoku degerlerinin eleman elaman alinacagi fonksiyon

void getValuesOneByOne()
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

// Kullanicidan sudoku degerlerinin satir satir alinacagi fonksiyon

void getValuesRowByRow()
{
    int i = 0,j = 0;
    for (i; i < 9; i++)
    {
        char str_row[20];
        printf("%d'inci satirdaki elemanlari giriniz (x1 x2..x8 x9):",i+1);
        fgets(str_row,sizeof(str_row),stdin);
        // 9 rakam aralarinda 8 bosluk olacagi icin ilk 17 char'a bakmak yeterli
        for(j=0;j<17;j=j+2)
        {
            char val = str_row[j];
            sudoku_matrix[i][j/2] = atoi(&val);
        }
    }
}

// Sutunlarin kontrol edilecegi fonksiyon

void *checkColumn(void *param)
{
    pthread_mutex_lock(&lock);
    details *dets = (details *) param;
    // pointer deki bilgiler int degerlere ataniyor
    int row_index = dets->row_index;
    int col_index = dets->column_index;
    // Girilen degerlerin dogrulugu kontrol ediliyor
    // Eger row_index degeri 0 degilse tum sutun degerleri alinamaz
    // Eger col_index degeri 0=<col_index=<8 degilse tasma olur
    if(row_index != 0 || col_index < 0 || col_index > 8 )
    {
        printf("Girilen degerler yanlis satir: %d sutun: %d \n",row_index,col_index);
        pthread_mutex_unlock(&lock);
        return NULL;
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
            pthread_mutex_unlock(&lock);
            return NULL;
        }
        // Sutunda bir tekrar olup olmadigini kontrol ediyor
        else if(tempArray[sudoku_matrix[i][col_index]-1] == 1)
        {
            printf("%d'inci sutun uygun degil\n",col_index);
            pthread_mutex_unlock(&lock);
            return NULL;
        }
        // Eger yukardaki durumlara sahip degilse tempArray deki degerini
        // 1 yapiyoruzki bu degerin kullanildigini tutalim
        else
        {
            tempArray[sudoku_matrix[i][col_index] -1 ] = 1;
        }
    }
    // Eger yukardaki return'lardan etkilenmeden kodun bu kismina gelebildiyse
    // sutundaki degerler uygundur thread degerine 1 koyarak bu sutundaki
    // degerlerin uygun oldugunu tutuyoruz
    validation++;
    pthread_mutex_unlock(&lock);
    return NULL;
}

// Satirlarin kontrol edilecegi fonksiyon

void *checkRow(void *param)
{
    pthread_mutex_lock(&lock);
    details *det = (details*) param;
    // pointer deki bilgiler int degerlere ataniyor
    int row_index = det->row_index;
    int col_index = det->column_index;
    // Sutun indeksi 0 degilse satir basi degildir
    // Satir indeksi 1-9 kapali araliginda degilse tasma olur
    if(col_index != 0 || row_index < 0 || row_index > 8)
    {
        printf("Girilen degerler yanlis satir: %d sutun: %d \n",row_index,col_index);
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    // Satirdaki degerlerin tekrarini tutmak icin array
    int tempArray[9] = {0};
    int i = 0;
    for(i;i<9;i++)
    {
        // Degerin 1-9 kapali araliginda olup olmadigini kontrol ediyor
        if(sudoku_matrix[row_index][i] < 1 || sudoku_matrix[row_index][i] > 9)
        {
            printf("Gecersiz deger degerler [1-9] araliginda olabilir. Degeriniz : %d\n",sudoku_matrix[row_index][i]);
            pthread_mutex_unlock(&lock);
            return NULL;
        }
        // Satirda bir tekrar olup olmadigini kontrol ediyor
        else if(tempArray[sudoku_matrix[row_index][i] - 1] == 1)
        {
            printf("%d'inci satir uygun degil\n",row_index);
            pthread_mutex_unlock(&lock);
            return NULL;
        }
        // Eger yukardaki durumlara sahip degilse tempArray deki degerini
        // 1 yapiyoruzki bu degerin kullanildigini tutalim
        else
        {
            tempArray[sudoku_matrix[row_index][i] - 1] = 1;
        }
    }
    validation++;
    pthread_mutex_unlock(&lock);
    return NULL;
}

// Alt matrislerin kontrol edilecegi fonksiyon

void *checkSubMatrix(void *param)
{
    pthread_mutex_lock(&lock);
    
    details *det = (details*) param;
    // pointer deki bilgiler int degerlere ataniyor
    int row_index = det->row_index;
    int col_index = det->column_index;
    // Alt matrislerin sol ust kosesindeki indeksleri ile islem yapilacak
    // gelen degerlerin uygun olup olmadigi kontrol ediliyor
    if(row_index < 0 || row_index > 6 || row_index % 3 != 0 || col_index < 0 || col_index > 6 || col_index % 3 != 0)
    {
        printf("Gecersiz satir ya da sutun index degeri! satir: %d sutun: %d",row_index,col_index);
        pthread_mutex_unlock(&lock);
        return NULL;
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
                pthread_mutex_unlock(&lock);
                return NULL;
            }
            else if (tempArray[sudoku_matrix[i][j] - 1] == 1)
            {
                printf("%d'inci alt matriks uygun degil\n",(col_index/3 + row_index + 1));
                pthread_mutex_unlock(&lock);
                return NULL;
            }
            else
            {
                tempArray[sudoku_matrix[i][j] - 1] = 1;
            }
               
        }
    }
    validation++;
    pthread_mutex_unlock(&lock);
    return NULL;
}
