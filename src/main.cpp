#include "external/crow.h"
#include <iostream>

int main() {
    crow::SimpleApp app;

    // Rota inicial: responde ao acessar http://seu-ip/
    CROW_ROUTE(app, "/")([](){
        return "Conexão estabelecida com o cpp_web_playground!";
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