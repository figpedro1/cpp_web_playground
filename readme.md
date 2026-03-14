# Instruções de execução do projeto

## 1) Pré-requisitos
- Linux (Ubuntu/Debian recomendado)
- Compilador C++ (g++/clang++)
- CMake 3.10+
- Boost (módulo system) ou Asio standalone
- Node.js 18+ e npm/yarn

## 2) Build e execução do backend C++ (servidor Crow)
1. No diretório raiz do projeto:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

2. Execute o servidor:

```bash
sudo ./cpp_web_playground
```

> O servidor inicia na porta 80, acesse `http://localhost/` e `http://localhost/ping`.

## 3) Build e execução do frontend (Vite + Vue)
1. Instale dependências front-end:

```bash
cd web
npm install
```

2. Rode em modo dev:

```bash
npm run dev
```

> Depois de iniciar, o Vite mostra a URL local (geralmente `http://localhost:5173`).

3. Para build de produção (opcional):

```bash
npm run build
npm run preview
```

## 4) Dependências completas
- Backend:
  - `crow` (header em `external/crow.h`)
  - Boost::system
  - Threads (pthreads)
- Frontend:
  - `vue`
  - `@types/node`
  - `@vitejs/plugin-vue`
  - `@vue/tsconfig`
  - `typescript`
  - `vite`
  - `vue-tsc`

## 5) Dicas rápidas
- Se `sudo` for necessário para usar porta 80, use `sudo` para executar o binário.
- Para mudar porta, edite `src/main.cpp` e use `.port(8080)`.
