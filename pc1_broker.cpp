#include <zmq.hpp>
#include <iostream>
#include <thread>

// Función del Hilo trabajador (Worker) para el Broker
void broker_worker(zmq::context_t* context) {
    // XSUB recibe los datos de los sensores locales
    zmq::socket_t frontend(*context, ZMQ_XSUB);
    frontend.bind("tcp://127.0.0.1:5555");
    
    // XPUB publica los datos hacia la red ZeroTier (hacia PC2)
    zmq::socket_t backend(*context, ZMQ_XPUB);
    backend.bind("tcp://10.147.20.1:5556"); // <--- IP ZEROTIER DEL PC1
    
    std::cout << "[HILO BROKER] Enrutando mensajes (PUB/SUB) en segundo plano..." << std::endl;
    // Proxy bloqueante, pero aislado en su propio hilo
    zmq::proxy(frontend, backend, nullptr); 
}

int main() {
    zmq::context_t context(1);
    std::cout << "[MAIN PC1] Iniciando Broker ZMQ Multihilo..." << std::endl;
    
    // Lanzamos el broker en un hilo independiente (Diap. 102)
    std::thread hilo_broker(broker_worker, &context);
    
    // El hilo principal queda libre (evita inanición)
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        std::cout << "[MAIN PC1] Broker funcionando correctamente." << std::endl;
    }
    
    hilo_broker.join();
    return 0;
}
