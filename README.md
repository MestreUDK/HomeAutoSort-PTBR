# Home Auto Sort 2.4 — PT-BR localization build

Projeto de desenvolvimento para compilar uma versão localizada do **Home Auto Sort 2.4**.

> Este repositório **não é o mod original**. Ele contém o código-fonte disponibilizado pelo autor do Home Auto Sort, com uma alteração focada em externalizar textos da interface para `HomeAutoSort_Translation.ini`, permitindo a tradução completa para PT-BR.

## Objetivo

A versão 2.4 possui diversos textos de interface diretamente dentro de `HomeAutoSort.dll`, inclusive menus, categorias, tooltips e os prompts do SkyPrompt. Esta adaptação adiciona uma pequena camada de localização e mantém textos ingleses como fallback caso uma chave de tradução esteja ausente.

Exemplos:

- `Master Chest` → `Baú Principal`
- `Written Works` → `Obras Escritas`
- `Notes / Letters / Journals` → `Notas / Cartas / Diários`
- `Hold to Stash` → `Segure para Guardar`
- `Hold to Resupply` → `Segure para Reabastecer`

## Compilar pelo GitHub Actions

1. Crie um repositório no GitHub.
2. Extraia o conteúdo deste pacote e envie **o conteúdo da pasta**, preservando `.github/workflows/build.yml`.
3. Faça o commit para a branch `main`.
4. Abra a aba **Actions** do repositório.
5. Selecione **Build Home Auto Sort PT-BR**.
6. O workflow deve iniciar automaticamente após o push. Também é possível usar **Run workflow**.
7. Quando terminar, abra a execução e baixe o artifact **HomeAutoSort-PTBR-2.4**.
8. Dentro dele haverá `HomeAutoSort_PTBR_2.4_GitHubBuild.zip`, pronto para teste no MO2.

## Como o Actions compila

O workflow usa um runner Windows e:

1. instala XMake 3.1.1;
2. baixa CommonLibSSE-NG v9.1.0;
3. baixa os headers oficiais atuais das APIs do SKSE Menu Framework e SkyPrompt;
4. usa a configuração universal do CommonLibSSE-NG para aproveitar o bundle pré-compilado quando disponível;
5. compila `HomeAutoSort.dll` em modo `releasedbg`;
6. cria um ZIP para MO2 contendo a DLL e a tradução PT-BR.

## Instalação para teste

Instale primeiro o **Home Auto Sort 2.4 original**, com seus requisitos. Depois instale o ZIP gerado por este projeto abaixo dele no painel esquerdo do Mod Organizer 2.

Ordem esperada:

```text
Home Auto Sort 2.4
Home Auto Sort - PT-BR 2.4 TESTE
```

O pacote gerado sobrescreve apenas:

```text
SKSE/Plugins/HomeAutoSort.dll
SKSE/Plugins/HomeAutoSort_Translation.ini
```

Ele não inclui nem substitui o arquivo pessoal `HomeAutoSort.ini` do usuário.

## Requisitos em jogo

Use os requisitos indicados pelo Home Auto Sort original. Para esta versão 2.4, o código integra-se com:

- SKSE;
- Address Library / CommonLibSSE-NG runtime support;
- SKSE Menu Framework;
- SkyPrompt.

## O que testar

Antes de qualquer publicação, valide pelo menos:

- Skyrim inicia sem crash antes do menu principal;
- Home Auto Sort aparece no menu do SKSE Menu Framework;
- General Settings / Configurações Gerais abre normalmente;
- Cell 1–5 funciona;
- busca e seleção de containers funciona;
- Master Chest / Baú Principal funciona;
- Stash / Guardar funciona;
- Resupply / Reabastecer funciona;
- prompts do SkyPrompt aparecem em PT-BR;
- salvar, sair do jogo e recarregar mantém as configurações;
- nenhum item ou preset existente é perdido.

## Dependências de compilação

Não ficam armazenadas neste repositório. O workflow as obtém durante o build:

- CommonLibSSE-NG `v9.1.0`;
- SimpleIni `v4.25` via XMake;
- SKSE Menu Framework API;
- SkyPrompt API.

## Créditos

- **zfroggyman** — autor original do Home Auto Sort.
- **QTR Modding / Quantumyilmaz** — APIs do SKSE Menu Framework e SkyPrompt.
- **CommonLibSSE-NG contributors** — framework para plugins SKSE.
- **MestreUDK** — tradução PT-BR, adaptação de localização e testes.

## Licença

O código derivado mantém os arquivos `COPYING.txt` e `EXCEPTIONS.md` fornecidos junto ao código-fonte do Home Auto Sort. Consulte-os antes de redistribuir builds binários ou código modificado.
