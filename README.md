# Índice B+ em C++

Este projeto implementa um índice B+ em C++ para a disciplina de Sistemas de Bancos de Dados (CK0117 - 2025.1), com suporte a inserções e buscas com base no atributo `ano_colheita` de vinhos armazenados em um arquivo CSV. O índice respeita a restrição de uso de apenas uma página de dados e uma de índice na memória por vez.

## 📁 Estrutura dos Arquivos

- `vinhos.csv` — Arquivo de dados contendo os registros de vinhos (id, rotulo, ano_colheita, tipo).
- `in.txt` — Arquivo de entrada com comandos a serem processados.
- `out.txt` — Arquivo de saída contendo os resultados dos comandos processados.
- `indice.txt` — Arquivo onde o índice B+ é armazenado de forma persistente.
- `main.cpp` — Código-fonte do projeto.
- `README.md` — Este arquivo de documentação.

## 📌 Formato dos Arquivos

### `vinhos.csv`

```csv
id,rotulo,ano_colheita,tipo
1,Cabernet,2012,Tinto
2,Merlot,2014,Tinto
...
```

### `in.txt`

A primeira linha define a ordem da árvore B+:

```
FLH/3
```

As linhas seguintes contêm comandos de inserção (`INC:`) ou busca (`BUS=:`), por exemplo:

```
INC:2012/
BUS=:2012/
```

### `out.txt`

Contém os resultados dos comandos, indicando a quantidade de inserções ou buscas bem-sucedidas:

```
FLH/3
INC:2012/1
BUS=:2012/1
```

## ⚙️ Execução

1. Compile o código:

```bash
g++ main.cpp -o bplus
```

2. Certifique-se de que os arquivos `vinhos.csv` e `in.txt` estejam no mesmo diretório.
3. Execute o programa:

```bash
./bplus
```

4. O resultado estará no arquivo `out.txt` e o índice persistido em `indice.txt`.

## 🧠 Lógica de Funcionamento

- O índice é implementado com estrutura de árvore B+, com nós internos e folhas serializados no arquivo `indice.txt`.
- Cada inserção (`INC:`) verifica se o valor já está no índice:
  - Se não estiver, busca os IDs no `vinhos.csv`, insere no índice e registra a operação no `out.txt`.
  - Se já estiver, não insere novamente.
- Cada busca (`BUS=:`) retorna a quantidade de IDs associados ao `ano_colheita`.

## 🗃️ Serialização do Índice

Cada nó do índice é armazenado no arquivo `indice.txt` no seguinte formato:

```text
<id>,<tipo>,[<chaves>],<pai>,<prev>,<next>,[<refs ou filhos>]
```

Exemplo de nó folha:

```
1,'folha',[2012,2014],null,null,2,[[1],[2]]
```

## 🔍 Limitações

- Apenas uma página de dados e uma de índice são mantidas na memória por vez.
- A árvore não suporta remoções.
- As inserções não atualizam registros existentes com ano duplicado (evitam reinserção).

## 👨‍💻 Autor

Implementado como parte da disciplina **CK0117 - Sistemas de Bancos de Dados (2025.1)**.
