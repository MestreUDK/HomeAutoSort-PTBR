# Home Auto Sort 2.4 — Tradução PT-BR

Tradução e adaptação de localização para **Home Auto Sort 2.4**, de **zfroggyman**, com suporte a textos externos em PT-BR por meio do arquivo `HomeAutoSort_Translation.ini`.

> [!IMPORTANT]
> Este repositório **não é o mod original** e não pretende substituir o trabalho do autor.
> O **Home Auto Sort 2.4 original é obrigatório**. Este projeto distribui uma versão modificada da DLL exclusivamente para permitir a localização completa da interface em português brasileiro.

## 📌 Informações do projeto

| Item | Informação |
| --- | --- |
| Mod base | Home Auto Sort 2.4 |
| Autor original | zfroggyman |
| Tradução / adaptação PT-BR | MestreUDK |
| Idioma | Português do Brasil |
| Status | Funcional e testado em jogo |
| Build | GitHub Actions + XMake |
| Licença do código derivado | GPL-3.0-or-later |

## 🔗 Links

- **Autor original — zfroggyman:** [Perfil no Nexus Mods](https://www.nexusmods.com/profile/zfroggyman)
- **Mod original — Home Auto Sort:** [Nexus Mods](https://www.nexusmods.com/skyrimspecialedition/mods/183379)
- **Código-fonte desta adaptação:** [HomeAutoSort-PTBR no GitHub](https://github.com/MestreUDK/HomeAutoSort-PTBR)

## 🇧🇷 Sobre esta tradução

O Home Auto Sort possui vários textos de interface diretamente dentro de `HomeAutoSort.dll`, incluindo menus, categorias, tooltips e prompts utilizados pelo SkyPrompt.

Esta adaptação adiciona uma pequena camada de localização para que esses textos sejam lidos de:

```text
SKSE/Plugins/HomeAutoSort_Translation.ini
```

Quando uma chave de tradução não é encontrada, o texto original em inglês é utilizado como fallback.

Exemplos de textos localizados:

- `Master Chest` → `Baú Principal`
- `Written Works` → `Obras Escritas`
- `Notes / Letters / Journals` → `Notas / Cartas / Diários`
- `Hold to Stash` → `Segure para Guardar`
- `Hold to Resupply` → `Segure para Reabastecer`

A adaptação é focada em **localização**. A lógica e as funcionalidades do Home Auto Sort continuam sendo trabalho do autor original.

## ✅ Status dos testes

A build PT-BR foi compilada com sucesso pelo GitHub Actions e testada em jogo junto ao Home Auto Sort 2.4 original.

Foram verificados:

- carregamento da DLL pelo SKSE;
- inicialização normal do Skyrim;
- exibição do Home Auto Sort no SKSE Menu Framework;
- carregamento da interface em PT-BR;
- funcionamento dos menus de configuração;
- funcionamento dos prompts localizados;
- funcionamento do sistema de guardar itens (`Stash`);
- funcionamento das funções principais utilizadas durante o teste.

O pacote testado mantém o mod original instalado e utiliza esta adaptação com prioridade maior no gerenciador de mods.

## 📦 Requisitos

Instale o **Home Auto Sort 2.4 original** e todos os requisitos indicados na página oficial do mod.

Entre as integrações utilizadas pelo Home Auto Sort estão:

- SKSE;
- SKSE Menu Framework;
- SkyPrompt.

Consulte sempre a página oficial do Home Auto Sort para verificar os requisitos e versões atualmente recomendados.

## 🛠️ Instalação

### Mod Organizer 2

1. Instale o **Home Auto Sort 2.4 original** e seus requisitos.
2. Instale o arquivo ZIP da tradução PT-BR.
3. No painel esquerdo do MO2, deixe a tradução **abaixo do mod original**, para que os arquivos desta adaptação tenham prioridade.
4. Inicie o Skyrim pelo SKSE.

Ordem recomendada:

```text
Home Auto Sort 2.4
Home Auto Sort 2.4 - PT-BR
```

O pacote PT-BR substitui/adiciona apenas:

```text
SKSE/Plugins/HomeAutoSort.dll
SKSE/Plugins/HomeAutoSort_Translation.ini
```

O projeto **não inclui nem substitui** o arquivo pessoal:

```text
SKSE/Plugins/HomeAutoSort.ini
```

Portanto, as configurações pessoais do usuário não fazem parte do pacote desta tradução.

## ⚠️ Compatibilidade e suporte

Esta tradução foi desenvolvida especificamente sobre o código-fonte do **Home Auto Sort 2.4**.

Atualizações futuras do mod original podem alterar a DLL, os textos ou a estrutura interna do projeto. Nesse caso, uma nova adaptação poderá ser necessária.

Para problemas relacionados:

- **à tradução PT-BR ou à DLL desta adaptação:** utilize este repositório;
- **ao funcionamento do Home Auto Sort original:** consulte a página oficial do mod e o autor original.

Não solicite suporte ao autor original por problemas causados exclusivamente por esta versão modificada.

## 🔨 Compilação

O projeto pode ser compilado automaticamente pelo GitHub Actions por meio do workflow:

```text
.github/workflows/build.yml
```

O workflow utiliza um runner Windows e:

1. instala o XMake;
2. obtém o CommonLibSSE-NG e as dependências necessárias;
3. obtém os headers utilizados pelo SKSE Menu Framework e SkyPrompt;
4. configura o projeto;
5. compila `HomeAutoSort.dll`;
6. monta um pacote ZIP na estrutura esperada pelo Mod Organizer 2.

Após uma execução bem-sucedida, o Artifact contém o pacote gerado para instalação.

## 🧩 Dependências de compilação

As dependências de terceiros não precisam ficar armazenadas diretamente neste repositório quando são obtidas automaticamente pelo workflow.

A build utiliza, entre outros componentes:

- CommonLibSSE-NG;
- SimpleIni;
- SKSE Menu Framework API;
- SkyPrompt API;
- XMake;
- toolchain MSVC disponibilizada pelo runner Windows do GitHub Actions.

As versões efetivamente utilizadas pelo build devem ser consultadas no `xmake.lua` e no workflow do GitHub Actions.

## 📁 Estrutura principal

```text
.github/
└── workflows/
    └── build.yml

include/
src/
translations/
└── HomeAutoSort_Translation.ini

COPYING.txt
EXCEPTIONS.md
README.md
xmake.lua
```

## 🙏 Créditos

### Home Auto Sort

- **zfroggyman** — criador e autor original do **Home Auto Sort**.

Todo o mérito pelas funcionalidades originais, lógica de autosort, sistema de containers, presets, Stash, Resupply e demais recursos do mod pertence ao autor original.

### APIs e frameworks

- **SkyrimThiago** — SKSE Menu Framework.
- **Quantumyilmaz** — SkyPrompt.
- **alandtse e contribuidores do CommonLibSSE-NG** — CommonLibSSE-NG e infraestrutura utilizada por plugins SKSE.
- Demais autores e contribuidores das bibliotecas utilizadas pelas dependências do projeto.

### Tradução PT-BR

- **MestreUDK** — tradução para português brasileiro, externalização/localização dos textos, adaptação para compilação, GitHub Actions e testes da versão PT-BR.

## 📜 Licença

O **Home Auto Sort** é disponibilizado pelo autor original sob **GNU General Public License v3.0 or later (GPL-3.0-or-later)**, juntamente com exceções de linking aplicáveis ao ecossistema de modding.

Como este projeto é uma obra derivada do código-fonte do Home Auto Sort, o código derivado deste repositório permanece sob **GPL-3.0-or-later**, respeitando os avisos e condições originais.

Consulte:

```text
COPYING.txt
EXCEPTIONS.md
```

Esses arquivos devem ser preservados em redistribuições do código ou de builds derivados quando aplicável.

A presença deste repositório, da tradução ou de uma build modificada **não transfere a autoria do Home Auto Sort**. O autor original permanece devidamente creditado.

## ❤️ Agradecimentos

Agradecimentos especiais ao **zfroggyman** por desenvolver o Home Auto Sort, disponibilizar seu código-fonte e permitir modificações e redistribuições conforme os termos e permissões do projeto.

Esta tradução foi feita de fã para fã, com o objetivo de tornar o mod mais acessível para jogadores brasileiros.
