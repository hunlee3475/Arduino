#include <iostream>			// C++ = stdio.h
#include <string>			// C++ old style = string.h
#include <vector>			// vector 자료구조
#include <array>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>		// mutex
#include "SerialPort.cpp"	// Serial 통신을 위한 라이브러리

void* task(void*);			// thread 함수 => task main
void* taskSerial(void*);	// thread 함수 => Serial 전송용 thread
static pthread_mutex_t mutex;	// key
static std::vector<int> client_socket_fds;	// 클라이언트들의 fd 저장하는 자료구조
static int serial_port = 0;	// 시리얼포트 저장용 변수

// 네트워크상에서 문자열 저장 array
static std::array<char, BUFSIZ> tcp_message;		// buffer -> 임계영역
static std::array<char, BUFSIZ> serial_message;	// buffer -> 임계영역

int main(int argc, const char* argv[]){
	int server_sock_fd{0};
	int client_sock_fd{0};
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	pthread_t pid;
	if(argc != 2){
		std::cout << "./SERVER 0000" << std::endl;
		exit(EXIT_FAILURE);	// exit(1);
	}
	serial_port = serialport_init("/dev/ttyACM0", 115200, nullptr);
	if(serial_port == -1){
		std::cout << "Serial connect error()" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "Serial Port is connected..." << std::endl;
	
	server_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(server_sock_fd == -1){
		std::cout << "socket() error" << std::endl;
		exit(EXIT_FAILURE);
	}
	memset(&server_addr, 0, sizeof server_addr);
	memset(&client_addr, 0, sizeof client_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));	// port
	const int bind_state = bind(server_sock_fd, (const struct sockaddr*)&server_addr, sizeof server_addr);
	if(bind_state == -1){
		std::cout << "bind() error" << std::endl;
		exit(EXIT_FAILURE);
	}
	const int listen_state = listen(server_sock_fd, 5);
	if(listen_state == -1){
		std::cout << "listen() error" << std::endl;
		exit(EXIT_FAILURE);
	}
	// 온도, 습도 값들을 출력하는 thread 생성
	pthread_create(&pid, nullptr, taskSerial, static_cast<void*>(nullptr));
	socklen_t client_sock_addr_size {0ul};
	while(true){
		client_sock_addr_size = sizeof client_addr;
		client_sock_fd = accept(server_sock_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_sock_addr_size);
		if(client_sock_fd == -1){
			std::cout << "accept() error" <<std::endl;
			break;
		}
		std::cout << "Connected from client IP..." << inet_ntoa(client_addr.sin_addr) << std::endl;
		// 임계 영역
		pthread_mutex_lock(&mutex);		// 락 걸기
		client_socket_fds.push_back(client_sock_fd);	// vector
		pthread_mutex_unlock(&mutex);	// 락 풀기
		pthread_create(&pid, nullptr, task, static_cast<void*>(&client_sock_fd));
	}
	close(server_sock_fd);
	return int(0);
}



void* taskSerial(void*arg){
	while(true){
		const int serial_state {serialport_read_until(serial_port, serial_message.data(), '\n')};
		// 0 성공
		if(!serial_state){
			pthread_mutex_lock(&mutex);
			for(auto fd : client_socket_fds){
				write(fd, serial_message.data(), strlen(serial_message .data()));
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

void* task(void* arg){
	const int clnt_sock_fd {*(static_cast<int*>(arg))};
	int tcp_str_length {0};
	while((tcp_str_length = read(clnt_sock_fd, tcp_message.data(), BUFSIZ)) != 0){
		pthread_mutex_lock(&mutex);
		serialport_write(serial_port, tcp_message.data());
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_lock(&mutex);
	
	for(auto it {client_socket_fds.begin()}; it != client_socket_fds.end(); ++it) {
		if(*it == clnt_sock_fd){
			client_socket_fds.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
	close(clnt_sock_fd);
	return nullptr;
}


