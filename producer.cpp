// Solução do problema do produtor consumidor utilizando variáveis de condição
#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>
#include <array>
#include <vector>
#include <cassert>

// Mutex para proteger os recursos compartilhados
std::mutex m;
// Variável de condição que indica que existe espaço disponível no buffer
// O consumidor utiliza essa variável de condição para notificar o produtor que a fila não está cheia
std::condition_variable space_available;
// Variável de condição que indica que existem dados disponíveis no buffer
// O produtor utiliza essa variável de condição para notificar o consumidor que a fila não está vazia
std::condition_variable data_available;


// Numero de threads produtoras e consumidoras a serem criadas no main()
static const unsigned NUM_PRODUCERS = 1;
static const unsigned NUM_CONSUMERS = 10;


//  =========================  Circular buffer  ============================================
// Código que implementa um buffer circular
static const unsigned BUFFER_SIZE = 1000;
int buffer[BUFFER_SIZE];
static unsigned counter = 0;
unsigned in = 0, out = 0;

void add_buffer(int i)
{
  buffer[in] = i;
  in = (in+1) % BUFFER_SIZE;
  counter++;
}

int get_buffer()
{
  int v;
  v = buffer[out];
  out = (out+1) % BUFFER_SIZE;
  counter--;
  return v;
}
//  ==========================================================================================

// Altere esse valor caso queira que o produtor e consumidor durmam SLEEP_TIME milissegundos
// após cada iteração 
static const unsigned SLEEP_TIME = 0; // ms


// Producer
void producer_func(const unsigned id)
{
	int i = 0;
	while (true)		
	{
		// Cria um objeto do tipo unique_lock que no construtor chama m.lock()
		std::unique_lock<std::mutex> lock(m);

		// Verifica se o buffer está cheio e, caso afirmativo, espera notificação de "espaço disponível no buffer"
		while (counter == BUFFER_SIZE)
		{			
			space_available.wait(lock); // Espera por espaço disponível 
			// Lembre-se que a função wait() invoca m.unlock() antes de colocar a thread em estado de espera para que o consumidor consiga adquirir a posse do mutex m	e consumir dados
			// Quando a thread é acordada, a função wait() invoca m.lock() retomando a posse do mutex m
		}

		// O buffer não está mais cheio, então produz um elemento
		add_buffer(i);
		std::cout << "Producer " << id << " - produced: " << i << " - Buffer counter: " << counter << std::endl;
		i++;
		
		// Notifica o consumirod que existem dados a serem consumidos no buffer
		data_available.notify_one();
		
		// (Opicional) dorme por SLEEP_TIME milissegundos
		if (SLEEP_TIME > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

		// Verifica a integridade do buffer, ou seja, verifica se o número de elementos do buffer (counter)
		// é menor ou igual a BUFFER_SIZE
		assert(counter <= BUFFER_SIZE);		
	} // Fim de escopo -> o objeto lock será destruído e invocará m.unlock(), liberando o mutex m
}

// Consumer
void consumer_func(const unsigned id)
{
	while (true)
	{
		// Cria um objeto do tipo unique_lock que no construtor chama m.lock()
		std::unique_lock<std::mutex> lock(m);
		
		// Verifica se o buffer está vazio e, caso afirmativo, espera notificação de "dado disponível no buffer"
		while (counter == 0)
		{
			data_available.wait(lock); // Espera por dado disponível
			// Lembre-se que a função wait() invoca m.unlock() antes de colocar a thread em estado de espera para que o produtor consiga adquirir a posse do mutex m e produzir dados
			// Quando a thread é acordada, a função wait() invoca m.lock() retomando a posse do mutex m
		}

		// O buffer não está mais vazio, então consome um elemento
		int i = get_buffer();
		std::cout << "Consumer " << id << " - consumed: " << i << " - Buffer counter: " << counter << std::endl;

		space_available.notify_one();
		
		// (Opicional) dorme por SLEEP_TIME milissegundo
		if (SLEEP_TIME > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

		// Verifica a integridade do buffer, ou seja, verifica se o número de elementos do buffer (counter)
		// é maior ou igual a zero
		assert(counter >= 0);
	}  // Fim de escopo -> o objeto lock será destruído e invocará m.unlock(), liberando o mutex m
}

int main()
{
	// Cria NUM_PRODUCER thread produtoras  e NUM_CONSUMER threads consumidoras
	std::vector<std::thread> producers;
	std::vector<std::thread> consumers;

	for (unsigned i =0; i < NUM_PRODUCERS; ++i)
	{
		producers.push_back(std::thread(producer_func, i));
	}
	for (unsigned i =0; i < NUM_CONSUMERS; ++i)
	{
		consumers.push_back(std::thread(consumer_func, i));
	}

	consumers[0].join();
}
