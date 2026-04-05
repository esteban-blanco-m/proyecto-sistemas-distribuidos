
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp> // Librería JSON

using json = nlohmann::json;

// Función que ejecuta cada Hilo (Simula una Calle completa de forma independiente)
void simular_calle(char fila, zmq::context_t* context) {
    zmq::socket_t publisher(*context, ZMQ_PUB);
    publisher.connect("tcp://127.0.0.1:5555"); // Se conecta al broker local
    
    std::string frentes[] = {"_CALLE", "_CARRERA"};

    while (true) {
        for (int col = 1; col <= 9; col++) { // M = 9 (Carreras)
            std::string inter = "INT_" + std::string(1, fila) + std::to_string(col);
            
            for(std::string frente : frentes) {
                // Simulación de valores aleatorios
                int vol = rand() % 25; 
                int vel = rand() % 60 + 10;
                
                // 1. Cámara (Volumen)
                json j_cam;
                j_cam["sensor_id"] = "CAM-" + inter + frente;
                j_cam["tipo_sensor"] = "camara";
                j_cam["interseccion"] = inter;
                j_cam["volumen"] = vol;
                j_cam["timestamp"] = "2026-02-09T15:20:00Z"; // Timestamp (Reloj físico)
                
                // 2. Espira (Conteo)
                json j_esp;
                j_esp["sensor_id"] = "ESP-" + inter + frente;
                j_esp["tipo_sensor"] = "espira_inductiva";
                j_esp["interseccion"] = inter;
                j_esp["vehiculos_contados"] = vol + (rand() % 5);
                j_esp["timestamp"] = "2026-02-09T15:20:00Z";
                
                // 3. GPS (Velocidad y Densidad)
                json j_gps;
                j_gps["sensor_id"] = "GPS-" + inter + frente;
                j_gps["tipo_sensor"] = "gps";
                j_gps["interseccion"] = inter;
                j_gps["velocidad_promedio"] = vel;
                j_gps["densidad"] = vol * 2;
                j_gps["timestamp"] = "2026-02-09T15:20:00Z";

                // Enviar asíncronamente (IPC)
                publisher.send(zmq::buffer("TOPIC_SENSOR " + j_cam.dump()), zmq::send_flags::none);
                publisher.send(zmq::buffer("TOPIC_SENSOR " + j_esp.dump()), zmq::send_flags::none);
                publisher.send(zmq::buffer("TOPIC_SENSOR " + j_gps.dump()), zmq::send_flags::none);
            }
        }
        // Escenario 1: Datos cada 10 segundos
        std::this_thread::sleep_for(std::chrono::seconds(10)); 
    }
}

int main() {
    zmq::context_t context(1);
    std::vector<std::thread> hilos_calles;
    
    // N = 6 (Calles de la A a la F)
    const char filas[] = {'A','B','C','D','E','F'}; 

    std::cout << "[PC1] Levantando hilos para 54 intersecciones (6x9)..." << std::endl;

    // Lanzar un hilo por cada calle (Paralelismo)
    for (char fila : filas) {
        hilos_calles.push_back(std::thread(simular_calle, fila, &context));
    }

    for (auto& t : hilos_calles) {
        t.join();
    }
    return 0;
}
