#include "include/external/crow.h"
#include <iostream>

int main() {
    crow::SimpleApp app;

    // Rota inicial: responde ao acessar http://seu-ip/
    CROW_ROUTE(app, "/")([](){
        return "Servidor está funcionando!!! Hospedado na AWS";
    });

    // Rota de teste: responde em http://seu-ip/ping
    CROW_ROUTE(app, "/ping")([](){
        return "pong";
    });

    std::cout << "Servidor iniciando na porta 80..." << std::endl;

    // Roda o servidor na porta 80 (padrão HTTP)
    app.port(80).multithreaded().run();

    return 0;
}