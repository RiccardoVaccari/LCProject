# Introduzione

Il progetto presenta due versioni divise in due cartelle: 
- **_Versione1.0_**: contiene l'implementazione dei 4 livelli richiesti dalla traccia. 
- **_Versione2.0_** : contiene l'implementazione dei 4 livelli richiesti dalla traccia con l'aggiunta di un livello _extra_ dove sono stati implementati ulteriori costrutti di iterazione. 

In ognuna delle due cartelle è presente `classTree.png` dedicato alla rappresentazione grafica della gerarchia delle classi. 

Sono presenti due `Makefile` dedicati rispettivamente alla creazione del compilatore `kcomp` e alla creazione degli eseguibili dei vari file test.

Seguono le classi aggiunte e le modifiche effettuate nei vari livelli. 

## Livello 1

### Classi Aggiunte
- **AssignmentAST**: classe dedicata agli assegnamenti.
- **GlobalVarAST**: classe dedicata alla creazione di variabili globali.
- **StmtAST**: classe base per tutti i nodi statement. 
### Modifiche
- **VariableExprAST**: modificata la classe dedicata alla rappresentazione di riferimenti a variabili a seguito dell'aggiunta delle variabili globali
- **VarBindingAST**: aggiunta la possibilità di definire una variabile senza inizializzare un valore, in quanto presente l'assegnamento. 
- **BlockAST**: ora i blocchi avranno come valore di ritorno l'ultima riga. 


## Livello 2
### Classi Aggiunte
- **IfStmtAST**: classe dedicata alla gestione dei controlli tramite i costrutti if/else.
- **FotStmtAST**: classe dedicata alla gestione dei cicli con costrutto for (_classico_).
- **VarOperation**: classe dedicata all'`init` del costrutto For. 
### Modifiche
- Ѐ stato aggiunto l'operatore binario ">" per completezza.
- Aggiunti a livello di parsing gli operatori di incremento e decremento: `++i`,`--i`. 


## Livello 3
### Modifiche
- **BinaryExprAST**  
Nel terzo livello non sono state aggiunte delle classi. É stata modificata la classe **BinaryExprAST** aggiungendo la gestione per gli operatori logici.
- Aggiunta a livello di parsing la possibilità utilizzare numeri negativi. 

## Livello 4
### Classi Aggiunte
- **BindingAST**: classe base per i nodi di binding (**VarBindingAST** e **ArrayBindingAST**).
- **ArrayBindingAST**: classe per la definizione (o inizializzazione) di un array.
- **ArrayExprAST**: classe per riferisi ad un elemento dell'array.

### Modifiche
**GlobalVarAST**  
Aggiunta la possibilità di creare array globali, per farlo non è stata creata alcuna classe, ma è stata modificata **GlobalVarAST**, già esistente.

## Livello Extra
Nel livello extra sono stati introdotti ulteriori costrutti di iterazione. Per farlo è stata creata una nuova classe base **LoopAST** da cui deriveranno tutte le nuove classi. I nuovi costrutti inseriti sono i seguenti:
- **While**  
    Per l'implementazione è stata creata una nuova classe **WhileStmtAST**;
- **Do while**  
    Per l'implementazione è stata creata una nuova classe **DoWhileStmtAST**;
- **For each**  
    Per l'implementazione è stata creata una nuova classe **ForEachStmtAST**. Questo costrutto è strutturato nel seguente modo `for (iterator in array){...}` e ha il compito di assegnare ad ogni ciclo, in ordine, un elemento dell'array `array` alla variabile `iterator`.