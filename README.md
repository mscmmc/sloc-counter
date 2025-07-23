# 🛠️ SLOC Counter

Contador de linhas de código em C++, que discrimina sua divisão entre código, comentários e documentação (Doxygen) por arquivo ou diretório.

---

## ✅ Funcionalidades principais

- Percorre recursivamente diretórios usando `<filesystem>`.
- Conta:
  - Linhas de código,
  - Linhas de comentário,
  - Linhas de documentação,
  - Linhas em branco.
- Exibe saída em tabela formatada, com percentual por tipo.
- Suporta opções via CLI (`-r`, `-s`, `-S`, `--help`).

---

## 🚀 Como usar

```bash
g++ -std=c++17 *.cpp -o sloc
./sloc [-r] <arquivos ou diretórios>
```
    -r: percorrer diretórios recursivamente.

    -s / -S: ordenar saída por SLOC ou outro critério.

    --help: exibe ajuda.

📚 Detalhes técnicos

    Linguagem: C++17, uso de <filesystem>, ifstream, e manipulação de strings.

    Estrutura organizada por funções e struct FileInfo.

    Tratamento de casos complexos, como:

        Comentários inline e multilinha,

        Comentários dentro de strings,

        Detecção de Doxygen.

👥 Autoria

    Este projeto foi desenvolvido por Marcel Setúbal Costa e Olive Oliveira Medeiros (veja author.md) durante a disciplina de LP1 na UFRN/DIMAp.
