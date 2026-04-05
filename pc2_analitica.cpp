#include <zmq.hpp>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    zmq::context_t context(1);

    // SUB: Lee del Broker en PC1
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://10.147.20.1:5556"); // <--- IP ZEROTIER DEL PC1
    subscriber.set(zmq::sockopt::subscribe, "TOPIC_SENSOR");

    // PUSH: Hacia BD Principal en PC3
    zmq::socket_t push_principal(context, ZMQ_PUSH);
    push_principal.connect("tcp://10.147.20.3:5557"); // <--- IP ZEROTIER DEL PC3

    // PUSH: Hacia BD Réplica local en PC2 (Enmascaramiento de fallas)
    zmq::socket_t push_replica(context, ZMQ_PUSH);
    push_replica.connect("tcp://127.0.0.1:5558"); // Localhost (PC2)

    std::cout << "[PC2] Analitica iniciada. Evaluando reglas de 54 intersecciones..." << std::endl;

    while (true) {
        zmq::message_t update;
        subscriber.recv(update, zmq::recv_flags::none);
        
        std::string msg = update.to_string();
        std::string json_str = msg.substr(13); // Quita el "TOPIC_SENSOR "
        
        try {
            json data = json::parse(json_str);
            
            // Regla básica: Detección de congestión
            if (data["tipo_sensor"] == "camara" && data["volumen"] > 20) {
                std::cout << "[CONGESTION] Detectada en " << data["sensor_id"] << std::endl;
            }

            // ENMASCARAMIENTO DE FALLAS (Tolerancia a fallos por redundancia)
            // send_flags::dontwait garantiza que si PC3 muere (Fail-stop), 
            // este proceso NO entra en Deadlock y sigue guardando en la Réplica.
            push_principal.send(zmq::buffer(json_str), zmq::send_flags::dontwait);
            push_replica.send(zmq::buffer(json_str), zmq::send_flags::dontwait);

        } catch (...) {
            std::cerr << "Error parseando evento." << std::endl;
        }
    }
    return 0;
}

