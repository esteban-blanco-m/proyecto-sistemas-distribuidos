#include <iostream>
#include <zmq.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

int main() {
    // Inicializar instancia de MongoDB (BD Distribuida)
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{"mongodb://localhost:27017"}};
    
    auto collection = conn["trafico_urbano"]["eventos"];

    // ZMQ PULL: Recibir datos asíncronos de PC2
    zmq::context_t context(1);
    zmq::socket_t puller(context, ZMQ_PULL);
    
    // bind en 0.0.0.0 permite escuchar por la IP de ZeroTier y Local
    puller.bind("tcp://0.0.0.0:5557"); 

    std::cout << "[PC3] Base de Datos Principal MongoDB iniciada. Escuchando en pto 5557..." << std::endl;

    while (true) {
        zmq::message_t message;
        // Recepción bloqueante (síncrona) para asegurar escritura local
        puller.recv(message, zmq::recv_flags::none); 
        std::string json_str = message.to_string();

        try {
            // Convierte el string JSON a BSON nativo de MongoDB y lo inserta
            auto bson_doc = bsoncxx::from_json(json_str);
            collection.insert_one(bson_doc.view());
            std::cout << "[PC3 BD] Evento guardado correctamente." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Error MongoDB] " << e.what() << std::endl;
        }
    }
    return 0;
}