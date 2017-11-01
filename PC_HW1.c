#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define DARTS 500000000
#define MASTER 0

int main (int argc, char *argv[]) {
	double dx, dy;			//Rastgele dartların koordinatları
	unsigned long hit = 0;		//Çeyrek çemberin içinde kalan noktalar
	unsigned long total_hits;	//Toplam hit sayısı
	int pid, np;	//Process id ve Number of process
	double  actual_pi = 3.141592;
	double pi;		//Hesaplayacağımız pi sayısı
	double elapsed_time = 0;	//Geçen süre
	
	//Burada kendim bir yöntem düşündüm. Eğer random seed değerini sadece process id verirsek,
	//her process için farklı ama processler içinde hep aynı rastgele sayılar gelir
	//Seed olarak zaman verirsek de, sadece saniye geçtikten sonra rastgele gelen sayılar değişir.
	//Ben de random hassasiyetini biraz daha arttırmak için, seed değerini
	//time + process id olarak verdim. Böylelikle hem her process farklı random sayılar gelecek,
	//hem de her saniye seed değişeceği için random sayılar her saniye tekrar farklılaşacak.
	//Yani daha rastgeleleşmiş bir algoritmamız olmuş olacak.
	time_t seconds;
	time(&seconds);
	srand((unsigned int) seconds + pid);	//Time ve process id'yi seed olarak alan srand
	
	MPI_Init(&argc, &argv);	//MPI başlangıcı
	MPI_Barrier(MPI_COMM_WORLD);	//Tüm processlerin by satıra ulaşmasını bekleyen fonksiyon
	//Geçen zamanı mpi_time'ın negatif değeri yapıyoruz. Sonrasında bunu yine time kadar artırınca, bu arada geçen zamanı elde etmiş olacağız.
	elapsed_time = -MPI_Wtime();
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);	//Process id
	MPI_Comm_size(MPI_COMM_WORLD, &np);		//Num. of process
	
	//Bu döngüde toplam iterasyon sayısını, process sayımıza bölüp, her process için
	//eşit sayıda iterasyon dağıtıyoruz. Mesela 1000 iterasyon olacaksa ve 4 process varsa,
	//1. process 1-250 arası, 2. process 251-500 arası iterasyonları hesaplayacaktır.
	for(int i = (pid)*(DARTS/np) + 1; i <= (pid+1)*(DARTS/np); i++) {
		dx = (double)rand() / (double)RAND_MAX;	//Rastgele x koordinatı
		dy = (double)rand() / (double)RAND_MAX;	//Rastgele y koordinatı
		//Eğer rastgele koordinatın orijine uzaklığı 1'den küçükse (çemberin içindeyse)
		//hit sayısını 1 arttır.
		if (sqrt(pow(dx,2) + pow(dy,2)) < 1) {
			hit = hit + 1;
		}
	}
	
	//Reduce fonksiyonu çok önemli olup, tüm processlerin hesapladığı hit sayısını, processlerin
	//hesaplama işlemleri bittikten sonra ana process'te toplar. Buradaki parametreler:
	//&hit: her processte olan ve işleme tabi tutulacak değişken, &total_hits: elde edilecek sonuç
	//1: işlemin tekrar sayısı, MPI_LONG: sonucun değişken tipi
	//MPI_SUM: işlem tipi, 0: ana process id'si
	MPI_Reduce(&hit, &total_hits, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	elapsed_time += MPI_Wtime();
	MPI_Finalize();
	
	pi = ((double)total_hits/(double)DARTS) * 4;
	if (pid == MASTER) {
		printf("Total hits: %ld in %d iterations.\n", total_hits, DARTS);
		printf("Pi is approximately: %.13lf\n", pi);
		printf("Elapsed time: %.4f seconds.\n", elapsed_time);
		printf("Presicion: %.13f\n", actual_pi - pi);
	}
	return 0;
}