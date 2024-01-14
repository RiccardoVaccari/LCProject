#include "driver.hpp"
#include "parser.hpp"

// Generazione di un'istanza per ciascuna della classi LLVMContext,
// Module e IRBuilder. Nel caso di singolo modulo è sufficiente
LLVMContext *context = new LLVMContext;
Module *module = new Module("Kaleidoscope", *context);
IRBuilder<> *builder = new IRBuilder(*context);

Value *LogErrorV(const std::string Str)
{
  std::cerr << Str << std::endl;
  return nullptr;
}

/* Il codice seguente sulle prime non è semplice da comprendere.
   Esso definisce una utility (funzione C++) con due parametri:
   1) la rappresentazione di una funzione llvm IR, e
   2) il nome per un registro SSA
   La chiamata di questa utility restituisce un'istruzione IR che alloca un double
   in memoria e ne memorizza il puntatore in un registro SSA cui viene attribuito
   il nome passato come secondo parametro. L'istruzione verrà scritta all'inizio
   dell'entry block della funzione passata come primo parametro.
   Si ricordi che le istruzioni sono generate da un builder. Per non
   interferire con il builder globale, la generazione viene dunque effettuata
   con un builder temporaneo TmpB
*/
static AllocaInst *CreateEntryBlockAlloca(Function *fun, StringRef VarName, Type *T = Type::getDoubleTy(*context))
{
  IRBuilder<> TmpB(&fun->getEntryBlock(), fun->getEntryBlock().begin());
  return TmpB.CreateAlloca(T, nullptr, VarName);
}

// Implementazione del costruttore della classe driver
driver::driver() : trace_parsing(false), trace_scanning(false){};

// Implementazione del metodo parse
int driver::parse(const std::string &f)
{
  file = f;                              // File con il programma
  location.initialize(&file);            // Inizializzazione dell'oggetto location
  scan_begin();                          // Inizio scanning (ovvero apertura del file programma)
  yy::parser parser(*this);              // Istanziazione del parser
  parser.set_debug_level(trace_parsing); // Livello di debug del parsed
  int res = parser.parse();              // Chiamata dell'entry point del parser
  scan_end();                            // Fine scanning (ovvero chiusura del file programma)
  return res;
}

// Implementazione del metodo codegen, che è una "semplice" chiamata del
// metodo omonimo presente nel nodo root (il puntatore root è stato scritto dal parser)
void driver::codegen()
{
  root->codegen(*this);
};

/************************* Sequence tree **************************/
SeqAST::SeqAST(RootAST *first, RootAST *continuation) : first(first), continuation(continuation){};

// La generazione del codice per una sequenza è banale:
// mediante chiamate ricorsive viene generato il codice di first e
// poi quello di continuation (con gli opportuni controlli di "esistenza")
Value *SeqAST::codegen(driver &drv)
{
  if (first != nullptr)
  {
    Value *f = first->codegen(drv);
  }
  else
  {
    if (continuation == nullptr)
      return nullptr;
  }
  Value *c = continuation->codegen(drv);
  return nullptr;
};

/********************* Number Expression Tree *********************/
NumberExprAST::NumberExprAST(double Val) : Val(Val){};

lexval NumberExprAST::getLexVal() const
{
  // Non utilizzata, Inserita per continuità con versione precedente
  lexval lval = Val;
  return lval;
};

// Non viene generata un'struzione; soltanto una costante LLVM IR
// corrispondente al valore float memorizzato nel nodo
// La costante verrà utilizzata in altra parte del processo di generazione
// Si noti che l'uso del contesto garantisce l'unicità della costanti
Value *NumberExprAST::codegen(driver &drv)
{
  return ConstantFP::get(*context, APFloat(Val));
};

/******************** Variable Expression Tree ********************/
VariableExprAST::VariableExprAST(const std::string &Name) : Name(Name){};

lexval VariableExprAST::getLexVal() const
{
  lexval lval = Name;
  return lval;
};

// NamedValues è una tabella che ad ogni variabile (che, in Kaleidoscope1.0,
// può essere solo un parametro di funzione) associa non un valore bensì
// la rappresentazione di una funzione che alloca memoria e restituisce in un
// registro SSA il puntatore alla memoria allocata. Generare il codice corrispondente
// ad una varibile equivale dunque a recuperare il tipo della variabile
// allocata e il nome del registro e generare una corrispondente istruzione di load
// Negli argomenti della CreateLoad ritroviamo quindi: (1) il tipo allocato, (2) il registro
// SSA in cui è stato messo il puntatore alla memoria allocata (si ricordi che A è
// l'istruzione ma è anche il registro, vista la corrispodenza 1-1 fra le due nozioni), (3)
// il nome del registro in cui verrà trasferito il valore dalla memoria
Value *VariableExprAST::codegen(driver &drv)
{
  AllocaInst *A = drv.NamedValues[Name];
  if (!A)
  {
    GlobalVariable *gVar = module->getNamedGlobal(Name);
    if (gVar != nullptr)
      return builder->CreateLoad(gVar->getValueType(), gVar, Name);
    else
      return LogErrorV("Variabile " + Name + " non definita");
  }
  return builder->CreateLoad(A->getAllocatedType(), A, Name.c_str());
}

/******************** Binary Expression Tree **********************/
BinaryExprAST::BinaryExprAST(char Op, ExprAST *LHS, ExprAST *RHS) : Op(Op), LHS(LHS), RHS(RHS){};

// La generazione del codice in questo caso è di facile comprensione.
// Vengono ricorsivamente generati il codice per il primo e quello per il secondo
// operando. Con i valori memorizzati in altrettanti registri SSA si
// costruisce l'istruzione utilizzando l'opportuno operatore
Value *BinaryExprAST::codegen(driver &drv)
{
  Value *L = LHS->codegen(drv);
  Value *R;
  if (RHS)
    R = RHS->codegen(drv);
  if (!L || (RHS && !R))
    return nullptr;
  switch (Op)
  {
  case '+':
    return builder->CreateFAdd(L, R, "addres");
  case '-':
    return builder->CreateFSub(L, R, "subres");
  case '*':
    return builder->CreateFMul(L, R, "mulres");
  case '/':
    return builder->CreateFDiv(L, R, "addres");
  case '<':
    return builder->CreateFCmpULT(L, R, "lttest");
  case '>':
    return builder->CreateFCmpUGT(L, R, "gttest");
  case '=':
    return builder->CreateFCmpUEQ(L, R, "eqtest");
  case 'a':
    return builder->CreateAnd(L, R, "andtest");
  case 'o':
    return builder->CreateOr(L, R, "ortest");
  case 'n':
    return builder->CreateNot(L, "nottest");
  default:
    std::cout << Op << std::endl;
    return LogErrorV("Operatore binario non supportato");
  }
};

/********************* Call Expression Tree ***********************/
/* Call Expression Tree */
CallExprAST::CallExprAST(std::string Callee, std::vector<ExprAST *> Args) : Callee(Callee), Args(std::move(Args)){};

lexval CallExprAST::getLexVal() const
{
  lexval lval = Callee;
  return lval;
};

Value *CallExprAST::codegen(driver &drv)
{
  // La generazione del codice corrispondente ad una chiamata di funzione
  // inizia cercando nel modulo corrente (l'unico, nel nostro caso) una funzione
  // il cui nome coincide con il nome memorizzato nel nodo dell'AST
  // Se la funzione non viene trovata (e dunque non è stata precedentemente definita)
  // viene generato un errore
  Function *CalleeF = module->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Funzione non definita");
  // Il secondo controllo è che la funzione recuperata abbia tanti parametri
  // quanti sono gi argomenti previsti nel nodo AST
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Numero di argomenti non corretto");
  // Passato con successo anche il secondo controllo, viene predisposta
  // ricorsivamente la valutazione degli argomenti presenti nella chiamata
  // (si ricordi che gli argomenti possono essere espressioni arbitarie)
  // I risultati delle valutazioni degli argomenti (registri SSA, come sempre)
  // vengono inseriti in un vettore, dove "se li aspetta" il metodo CreateCall
  // del builder, che viene chiamato subito dopo per la generazione dell'istruzione
  // IR di chiamata
  std::vector<Value *> ArgsV;
  for (auto arg : Args)
  {
    ArgsV.push_back(arg->codegen(drv));
    if (!ArgsV.back())
      return nullptr;
  }
  return builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/************************* Array Expression Tree *************************/
ArrayExprAST::ArrayExprAST(std::string Name, ExprAST *Offset) : Name(Name), Offset(Offset){};

Value *ArrayExprAST::codegen(driver &drv)
{

  Value *doubleIndex = Offset->codegen(drv);
  if (!doubleIndex)
    return nullptr;

  Value *floatIndex = builder->CreateFPTrunc(doubleIndex, Type::getFloatTy(*context));
  Value *intIndex = builder->CreateFPToSI(floatIndex, Type::getInt32Ty(*context));

  Value *A = drv.NamedValues[Name];
  if (!A)
  {
    A = module->getNamedGlobal(Name);
    if (!A)
      return LogErrorV("Variabile " + Name + " non definita");
  }

  Value *p = builder->CreateInBoundsGEP(Type::getDoubleTy(*context), A, intIndex);
  return builder->CreateLoad(Type::getDoubleTy(*context), p, Name.c_str());
};

/************************* If Expression Tree *************************/
IfExprAST::IfExprAST(ExprAST *Cond, ExprAST *TrueExp, ExprAST *FalseExp) : Cond(Cond), TrueExp(TrueExp), FalseExp(FalseExp){};

Value *IfExprAST::codegen(driver &drv)
{
  // Viene dapprima generato il codice per valutare la condizione, che
  // memorizza il risultato (di tipo i1, dunque booleano) nel registro SSA
  // che viene "memorizzato" in CondV.
  Value *CondV = Cond->codegen(drv);
  if (!CondV)
    return nullptr;

  // Ora bisogna generare l'istruzione di salto condizionato, ma prima
  // vanno creati i corrispondenti basic block nella funzione attuale
  // (ovvero la funzione di cui fa parte il corrente blocco di inserimento)
  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *TrueBB = BasicBlock::Create(*context, "trueexp", function);
  // Il blocco TrueBB viene inserito nella funzione dopo il blocco corrente
  BasicBlock *FalseBB = BasicBlock::Create(*context, "falseexp");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "endcond");
  // Gli altri due blocchi non vengono ancora inseriti perché le istruzioni
  // previste nel "ramo" true del condizionale potrebbe dare luogo alla creazione
  // di altri blocchi, che naturalmente andrebbero inseriti prima di FalseBB

  // Ora possiamo crere l'istruzione di salto condizionato
  builder->CreateCondBr(CondV, TrueBB, FalseBB);

  // "Posizioniamo" il builder all'inizio del blocco true,
  // generiamo ricorsivamente il codice da eseguire in caso di
  // condizione vera e, in chiusura di blocco, generiamo il saldo
  // incondizionato al blocco merge
  builder->SetInsertPoint(TrueBB);
  Value *TrueV = TrueExp->codegen(drv);
  if (!TrueV)
    return nullptr;
  builder->CreateBr(MergeBB);

  // Come già ricordato, la chiamata di codegen in TrueExp potrebbe aver inserito
  // altri blocchi (nel caso in cui la parte trueexp sia a sua volta un condizionale).
  // Ne consegue che il blocco corrente potrebbe non coincidere più con TrueBB.
  // Il branch alla parte merge deve però essere effettuato dal blocco corrente,
  // che dunque va recuperato. Ed è fondamentale sapere da quale blocco origina
  // il salto perché tale informazione verrà utilizzata da un'istruzione PHI.
  // Nel caso in cui non sia stato inserito alcun nuovo blocco, la seguente
  // istruzione corrisponde ad una NO-OP
  TrueBB = builder->GetInsertBlock();
  function->insert(function->end(), FalseBB);

  // "Posizioniamo" il builder all'inizio del blocco false,
  // generiamo ricorsivamente il codice da eseguire in caso di
  // condizione falsa e, in chiusura di blocco, generiamo il saldo
  // incondizionato al blocco merge
  builder->SetInsertPoint(FalseBB);

  Value *FalseV = FalseExp->codegen(drv);
  if (!FalseV)
    return nullptr;
  builder->CreateBr(MergeBB);

  // Esattamente per la ragione spiegata sopra (ovvero il possibile inserimento
  // di nuovi blocchi da parte della chiamata di codegen in FalseExp), andiamo ora
  // a recuperare il blocco corrente
  FalseBB = builder->GetInsertBlock();
  function->insert(function->end(), MergeBB);

  // Andiamo dunque a generare il codice per la parte dove i due "flussi"
  // di esecuzione si riuniscono. Impostiamo correttamente il builder
  builder->SetInsertPoint(MergeBB);

  // Il codice di riunione dei flussi è una "semplice" istruzione PHI:
  // a seconda del blocco da cui arriva il flusso, TrueBB o FalseBB, il valore
  // del costrutto condizionale (si ricordi che si tratta di un "expression if")
  // deve essere copiato (in un nuovo registro SSA) da TrueV o da FalseV
  // La creazione di un'istruzione PHI avviene però in due passi, in quanto
  // il numero di "flussi entranti" non è fissato.
  // 1) Dapprima si crea il nodo PHI specificando quanti sono i possibili nodi sorgente
  // 2) Per ogni possibile nodo sorgente, viene poi inserita l'etichetta e il registro
  //    SSA da cui prelevare il valore
  PHINode *PN = builder->CreatePHI(Type::getDoubleTy(*context), 2, "condval");
  PN->addIncoming(TrueV, TrueBB);
  PN->addIncoming(FalseV, FalseBB);
  return PN;
};

/********************** Block Expression Tree *********************/
BlockAST::BlockAST(std::vector<StmtAST *> Stmts) : Stmts(std::move(Stmts)){};

BlockAST::BlockAST(std::vector<BindingAST *> Def, std::vector<StmtAST *> Stmts) : Def(std::move(Def)), Stmts(std::move(Stmts)){};

Value *BlockAST::codegen(driver &drv)
{
  // Un blocco è un'espressione preceduta dalla definizione di una o più variabili locali.
  // Le definizioni sono opzionali e tuttavia necessarie perché l'uso di un blocco
  // abbia senso. Ad ogni variabile deve essere associato il valore di una costante o il valore di
  // un'espressione. Nell'espressione, arbitraria, possono chiaramente comparire simboli di
  // variabile. Al riguardo, la gestione dello scope (ovvero delle regole di visibilità)
  // è implementata nel modo seguente, in cui, come esempio, consideremo la definzione: var y = x+1
  // 1) Viene dapprima generato il codice per valutare l'espressione x+1.
  //    L'area di memoria da cui "prelevare" il valore di x è scritta in un
  //    registro SSA che è parte della (rappresentazione interna della) istruzione alloca usata
  //    per allocare la memoria corrispondente e che è registrata nella symbol table
  //    Per i parametri della funzione, l'istruzione di allocazione viene generata (come già sappiamo)
  //    dalla chiamata di codegen in FunctionAST. Per le variabili locali viene generata nel presente
  //    contesto. Si noti, di passaggio, che tutte le istruzioni di allocazione verranno poi emesse
  //    nell'entry block, in ordine cronologico rovesciato (rispetto alla generazione). Questo perché
  //    la routine di utilità (CreateEntryBlockAlloca) genera sempre all'inizio del blocco.
  // 2) Ritornando all'esempio, bisogna ora gestire l'assegnamento ad y gestendone la visibilità.
  //    Come prima cosa viene generata l'istruzione alloca per y.
  //    Questa deve essere inserita nella symbol table per futuri riferimenti ad y
  //    all'interno del blocco. Tuttavia, se un'istruzione alloca per y fosse già presente nella symbol
  //    table (nel caso y sia un parametro) bisognerebbe "rimuoverla" temporaneamente e re-inserirla
  //    all'uscita del blocco. Questo è ciò che viene fatto dal presente codice, che utilizza
  //    al riguardo il vettore di appoggio "AllocaTmp" (che naturalmente è un vettore di
  //    di (puntatori ad) istruzioni di allocazione
  std::vector<AllocaInst *> AllocaTmp;
  if (!Def.empty())
  {
    for (int i = 0; i < Def.size(); i++)
    {
      // Per ogni definizione di variabile si genera il corrispondente codice che
      // (in questo caso) non restituisce un registro SSA ma l'istruzione di allocazione
      AllocaInst *boundval = Def[i]->codegen(drv);
      if (!boundval)
        return nullptr;
      // Viene temporaneamente rimossa la precedente istruzione di allocazione
      // della stessa variabile (nome) e inserita quella corrente
      AllocaTmp.push_back(drv.NamedValues[Def[i]->getName()]);
      drv.NamedValues[Def[i]->getName()] = boundval;
    };
  }

  // Ora (ed è la parte più "facile" da capire) viene generato il codice che
  // valuta l'espressione. Eventuali riferimenti a variabili vengono risolti
  // nella symbol table appena modificata

  // Se non ho nessuna definizione (vardefs) dovrò soltanto occuparmi degli statements
  Value *blockvalue;
  for (int i = 0; i < Stmts.size(); i++)
  {
    blockvalue = Stmts[i]->codegen(drv);
    if (!blockvalue)
      return nullptr;
  }
  // Prima di uscire dal blocco, si ripristina lo scope esterno al costrutto
  if (!Def.empty())
  {
    for (int i = 0; i < Def.size(); i++)
    {
      drv.NamedValues[Def[i]->getName()] = AllocaTmp[i];
    };
  }
  // Il valore del costrutto/espressione var è ovviamente il valore (il registro SSA)
  // restituito dal codice di valutazione dell'espressione
  return blockvalue;
};

/************************* Binding Tree *************************/

void BindingAST::setName(std::string Name)
{
  this->Name = Name;
};

std::string BindingAST::getName() const
{
  return Name;
};

/************************* Var binding Tree *************************/
VarBindingAST::VarBindingAST(std::string Name, ExprAST *Val) : Val(Val) { setName(Name); };

AllocaInst *VarBindingAST::codegen(driver &drv)
{

  // Viene subito recuperato il riferimento alla funzione in cui si trova
  // il blocco corrente. Il riferimento è necessario perché lo spazio necessario
  // per memorizzare una variabile (ovunque essa sia definita, si tratti cioè
  // di un parametro oppure di una variabile locale ad un blocco espressione)
  // viene sempre riservato nell'entry block della funzione. Ricordiamo che
  // l'allocazione viene fatta tramite l'utility CreateEntryBlockAlloca
  Function *fun = builder->GetInsertBlock()->getParent();

  // Ora viene generato il codice che definisce il valore della variabile
  Value *BoundVal;
  if (Val)
  { // Val è nullptr quando ho una definizione senza allocazione (es. Var x invece che Var x = 2)
    BoundVal = Val->codegen(drv);
    if (!BoundVal) // Qualcosa è andato storto nella generazione del codice?
      return nullptr;
  }

  AllocaInst *Alloca = CreateEntryBlockAlloca(fun, Name);
  // Se tutto ok, si genera l'struzione che alloca memoria per la varibile ...
  // ... e si genera l'istruzione per memorizzarvi il valore dell'espressione,
  // ovvero il contenuto del registro BoundVal
  if (Val) // Val è nullptr quando ho una definizione senza allocazione (es. Var x invece che Var x = 2)
    builder->CreateStore(BoundVal, Alloca);
  // L'istruzione di allocazione (che include il registro "puntatore" all'area di memoria
  // allocata) viene restituita per essere inserita nella symbol table
  return Alloca;
};

/************************* Var binding Tree *************************/
ArrayBindingAST::ArrayBindingAST(std::string Name, double Size) : Size(Size) { setName(Name); };
ArrayBindingAST::ArrayBindingAST(std::string Name, double Size, std::vector<ExprAST *> Values) : Size(Size), Values(std::move(Values)) { setName(Name); };

AllocaInst *ArrayBindingAST::codegen(driver &drv)
{
  if (!Values.empty() && Values.size() > Size)
    return nullptr;

  Function *fun = builder->GetInsertBlock()->getParent();
  ArrayType *AT = ArrayType::get(Type::getDoubleTy(*context), Size);
  AllocaInst *Alloca = CreateEntryBlockAlloca(fun, Name, AT);

  std::vector<Value *> boundValues;

  if (!Values.empty())
  {
    for (int i = 0; i < Size; i++)
    {
      Value *boundValue;
      if (i >= Values.size())
        boundValue = Constant::getNullValue(Type::getDoubleTy(*context));
      else
      {
        boundValue = Values[i]->codegen(drv);
        if (!boundValue)
          return nullptr;
      }
      boundValues.push_back(boundValue);
    }

    // Per ogni valore, si crea il corrispondente index come Value*, si ottiene il rispettivo GEP e si effettua la Store.
    for (int i = 0; i < Size; i++)
    {
      Value *index = ConstantInt::get(*context, APInt(32, i, true));
      Value *p = builder->CreateInBoundsGEP(Type::getDoubleTy(*context), Alloca, index);
      builder->CreateStore(boundValues[i], p);
    }
  }

  return Alloca;
};

/************************* Prototype Tree *************************/
PrototypeAST::PrototypeAST(std::string Name, std::vector<std::string> Args) : Name(Name), Args(std::move(Args)), emitcode(true){}; // Di regola il codice viene emesso

lexval PrototypeAST::getLexVal() const
{
  lexval lval = Name;
  return lval;
};

const std::vector<std::string> &PrototypeAST::getArgs() const
{
  return Args;
};

// Previene la doppia emissione del codice. Si veda il commento più avanti.
void PrototypeAST::noemit()
{
  emitcode = false;
};

Function *PrototypeAST::codegen(driver &drv)
{
  // Costruisce una struttura, qui chiamata FT, che rappresenta il "tipo" di una
  // funzione. Con ciò si intende a sua volta una coppia composta dal tipo
  // del risultato (valore di ritorno) e da un vettore che contiene il tipo di tutti
  // i parametri. Si ricordi, tuttavia, che nel nostro caso l'unico tipo è double.

  // Prima definiamo il vettore (qui chiamato Doubles) con il tipo degli argomenti
  std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*context));
  // Quindi definiamo il tipo (FT) della funzione
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(*context), Doubles, false);
  // Infine definiamo una funzione (al momento senza body) del tipo creato e con il nome
  // presente nel nodo AST. ExternalLinkage vuol dire che la funzione può avere
  // visibilità anche al di fuori del modulo
  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, *module);

  // Ad ogni parametro della funzione F (che, è bene ricordare, è la rappresentazione
  // llvm di una funzione, non è una funzione C++) attribuiamo ora il nome specificato dal
  // programmatore e presente nel nodo AST relativo al prototipo
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  /* Abbiamo completato la creazione del codice del prototipo.
     Il codice può quindi essere emesso, ma solo se esso corrisponde
     ad una dichiarazione extern. Se invece il prototipo fa parte
     della definizione "completa" di una funzione (prototipo+body) allora
     l'emissione viene fatta al momendo dell'emissione della funzione.
     In caso contrario nel codice si avrebbe sia una dichiarazione
     (come nel caso di funzione esterna) sia una definizione della stessa
     funzione.
  */
  if (emitcode)
  {
    F->print(errs());
    fprintf(stderr, "\n");
  };

  return F;
}

/************************* Function Tree **************************/
FunctionAST::FunctionAST(PrototypeAST *Proto, StmtAST *Body) : Proto(Proto), Body(Body){};

Function *FunctionAST::codegen(driver &drv)
{
  // Verifica che la funzione non sia già presente nel modulo, cioò che non
  // si tenti una "doppia definizion"
  Function *function =
      module->getFunction(std::get<std::string>(Proto->getLexVal()));
  // Se la funzione non è già presente, si prova a definirla, innanzitutto
  // generando (ma non emettendo) il codice del prototipo
  if (!function)
    function = Proto->codegen(drv);
  else
    return nullptr;
  // Se, per qualche ragione, la definizione "fallisce" si restituisce nullptr
  if (!function)
    return nullptr;

  // Altrimenti si crea un blocco di base in cui iniziare a inserire il codice
  BasicBlock *BB = BasicBlock::Create(*context, "entry", function);
  builder->SetInsertPoint(BB);

  // Ora viene la parte "più delicata". Per ogni parametro formale della
  // funzione, nella symbol table si registra una coppia in cui la chiave
  // è il nome del parametro mentre il valore è un'istruzione alloca, generata
  // invocando l'utility CreateEntryBlockAlloca già commentata.
  // Vale comunque la pena ricordare: l'istruzione di allocazione riserva
  // spazio in memoria (nel nostro caso per un double) e scrive l'indirizzo
  // in un registro SSA
  // Il builder crea poi un'istruzione che memorizza il valore del parametro x
  // (al momento contenuto nel registro SSA %x) nell'area di memoria allocata.
  // Si noti che il builder conosce il registro che contiene il puntatore all'area
  // perché esso è parte della rappresentazione C++ dell'istruzione di allocazione
  // (variabile Alloca)

  for (auto &Arg : function->args())
  {
    // Genera l'istruzione di allocazione per il parametro corrente
    AllocaInst *Alloca = CreateEntryBlockAlloca(function, Arg.getName());
    // Genera un'istruzione per la memorizzazione del parametro nell'area
    // di memoria allocata
    builder->CreateStore(&Arg, Alloca);
    // Registra gli argomenti nella symbol table per eventuale riferimento futuro
    drv.NamedValues[std::string(Arg.getName())] = Alloca;
  }

  // Ora può essere generato il codice corssipondente al body (che potrà
  // fare riferimento alla symbol table)
  if (Value *RetVal = Body->codegen(drv))
  {
    // Se la generazione termina senza errori, ciò che rimane da fare è
    // di generare l'istruzione return, che ("a tempo di esecuzione") prenderà
    // il valore lasciato nel registro RetVal
    builder->CreateRet(RetVal);

    // Effettua la validazione del codice e un controllo di consistenza
    verifyFunction(*function);

    // Emissione del codice su su stderr)
    function->print(errs());
    fprintf(stderr, "\n");
    return function;
  }

  // Errore nella definizione. La funzione viene rimossa
  function->eraseFromParent();
  return nullptr;
};

/************************* GlobalVarAST **************************/
GlobalVarAST::GlobalVarAST(const std::string Name) : Name(Name){};
GlobalVarAST::GlobalVarAST(const std::string Name, int Size) : Name(Name), Size(Size){};

GlobalVariable *GlobalVarAST::codegen(driver &drv)
{
  Type *T;
  Constant *initValue;
  if (!Size)
  {
    // Caso Variabile
    T = Type::getDoubleTy(*context);                               // tipo double
    initValue = ConstantFP::get(Type::getDoubleTy(*context), 0.0); // valore iniziale a 0
  }
  else
  {
    // Caso Array
    T = ArrayType::get(Type::getDoubleTy(*context), Size); // tipo per l'array
    initValue = ConstantAggregateZero::get(T);             // valore iniziale che pone a 0 tutti gli elementi dell'array
  }

  GlobalVariable *globVar = new GlobalVariable(*module, T, false, GlobalValue::CommonLinkage, initValue, Name);
  globVar->print(errs());
  fprintf(stderr, "\n");

  return globVar;
}

/************************* AssignmentAST **************************/
AssignmentAST::AssignmentAST(std::string Name, ExprAST *AssignExpr) : Name(Name), AssignExpr(AssignExpr){};
AssignmentAST::AssignmentAST(std::string Name, ExprAST *OffsetExpr, ExprAST *AssignExpr) : Name(Name), OffsetExpr(OffsetExpr), AssignExpr(AssignExpr){};

Value *AssignmentAST::codegen(driver &drv)
{
  // Recupero dalla symbol table la variabile su cui si cerca di effettuare un assegnamento
  Value *A = drv.NamedValues[Name];
  if (!A)
  {
    A = module->getNamedGlobal(Name);
    if (!A)
      // Se non è definita nè nella symbol table nè globalmente allora restituirò errore
      return LogErrorV("Variabile " + Name + " non definita");
  }

  // Calcolo del RHS
  Value *RHS = AssignExpr->codegen(drv);
  if (!RHS)
    return nullptr;

  // Controllo che sia stato definito un offset,
  // Se è stato definito vuol dire che si sta
  // tentando di effettuare un assignment su una cella di un Array
  if (OffsetExpr)
  {
    // Calcolo l'offset, lo converto a intero, mediante CreateInBoundGep creo un riferimento e infine memorizzo l'RHS nel riferimento trovato
    Value *doubleIndex = OffsetExpr->codegen(drv);
    Value *floatIndex = builder->CreateFPTrunc(doubleIndex, Type::getFloatTy(*context));
    Value *intIndex = builder->CreateFPToSI(floatIndex, Type::getInt32Ty(*context));
    Value *p = builder->CreateInBoundsGEP(Type::getDoubleTy(*context), A, intIndex);
    builder->CreateStore(RHS, p);
  }
  else
    // semplice memorizzazione dell'RHS nel riferimento alla variabile
    builder->CreateStore(RHS, A);

  return RHS;
}

const std::string &AssignmentAST::getName() const
{
  return Name;
};

/************************* IfStmtAST **************************/
IfStmtAST::IfStmtAST(ExprAST *CondExpr, StmtAST *TrueStmt, StmtAST *ElseStmt) : CondExpr(CondExpr), TrueStmt(TrueStmt), ElseStmt(ElseStmt){};

Value *IfStmtAST::codegen(driver &drv)
{
  // Recupero il basicBB in cui si stava scrivendo precedentemente
  // per motivi legati al PhiNode finale (dopo spiegazione)
  BasicBlock *entryBB = builder->GetInsertBlock();

  // Viene generato il codice per il controllo della condizione
  Value *CondV = CondExpr->codegen(drv);
  if (!CondV)
    return nullptr;

  // vengono creati i tre BB ma allocato solo uno
  // TrueBB (allocato), FalseBB (presente solo se c'è un ramo else) e MergeBB
  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *TrueBB = BasicBlock::Create(*context, "truestmt", function);

  BasicBlock *FalseBB;
  if (ElseStmt)
    FalseBB = BasicBlock::Create(*context, "elsestmt");

  BasicBlock *MergeBB = BasicBlock::Create(*context, "endstmt");

  // Ramo Else presente -> salto a TrueBB se la condizione è vera, a FalseBB altrimenti
  // Ramo Else NON presente -> salto a TrueBB se la condizione è vera, a MergeBB (fine) altrimenti
  if (ElseStmt)
    builder->CreateCondBr(CondV, TrueBB, FalseBB);
  else
    builder->CreateCondBr(CondV, TrueBB, MergeBB);

  // Genero il codice per il TrueBB
  builder->SetInsertPoint(TrueBB);
  Value *TrueV = TrueStmt->codegen(drv);
  if (!TrueV)
    return nullptr;

  // Salto incodizionato vero il merge
  builder->CreateBr(MergeBB);

  // Ramo Else presente -> inserisco il FalseBB dopo il TrueBB
  // Ramo Else NON presente -> inserisco il MergeBB dopo il TrueBB
  TrueBB = builder->GetInsertBlock();
  if (ElseStmt)
    function->insert(function->end(), FalseBB);
  else
    function->insert(function->end(), MergeBB);

  // Se è presente il ramo Else...
  Value *FalseV;
  if (ElseStmt)
  {
    // Setto correttamente il punto di scrittura del builder...
    builder->SetInsertPoint(FalseBB);
    // e genero la condizione.
    FalseV = ElseStmt->codegen(drv);
    if (!FalseV)
      return nullptr;
    builder->CreateBr(MergeBB);

    // Inserisco il mergeBB dopo il falseBB
    FalseBB = builder->GetInsertBlock();
    function->insert(function->end(), MergeBB);
  }

  builder->SetInsertPoint(MergeBB);

  // Nel PhiNode (nel mergeBB)...
  PHINode *PN = builder->CreatePHI(Type::getDoubleTy(*context), 2, "condval");
  // Se si arriva dal TrueBB ci sarà il rispettivo Value.
  PN->addIncoming(TrueV, TrueBB);
  // Bisogna controllare che esista il ramo Else poichè:
  if (ElseStmt)
    // Se esiste allora, se la condizione è risultata falsa, arrivo dal FalseBB e ci sarà il rispettivo Value
    PN->addIncoming(FalseV, FalseBB);
  else
    // Se non esiste allora, se la condizione è risultata falsa, arrivo dall'entryBB (ciò da cui scrivevo prima) e ci sarà 0 come Value
    PN->addIncoming(Constant::getNullValue(Type::getDoubleTy(*context)), entryBB);

  return PN;
};

/*************************Var Operation*****+********************/

VarOperation::VarOperation(varOp operation) : operation(operation){};

varOp VarOperation::getOp()
{
  return operation;
};

/************************* ForStmtAST **************************/

ForStmtAST::ForStmtAST(VarOperation *InitExp, ExprAST *CondExpr, AssignmentAST *AssignExpr, StmtAST *BodyStmt) : InitExp(InitExp), CondExpr(CondExpr), AssignExpr(AssignExpr), BodyStmt(BodyStmt){};

Value *ForStmtAST::codegen(driver &drv)
{
  // Setto l'insertPoint dal BB da cui stavo scrivendo prima
  //  BasicBlock *entryBB = builder->GetInsertBlock();
  //  builder->SetInsertPoint(entryBB);

  // Generazione codice condizione per init.
  // Nell'init ci potranno essere due casi:
  //-> caso assignement -> InitExp è un assignment -> il suo codgen ritorna un Value
  //-> caso VarBinding -> InitExp è un varBinding -> il suo codgen ritorna un AllocaInst da usare opportunamente
  // tmpAlloca è una variabile che servirà per ripristinare lo scope esterno una volta terminato il ciclo.
  AllocaInst *tmpAlloca;
  if (InitExp->getOp().index())
  {
    // ASSIGNMENT
    Value *InitV = std::get<AssignmentAST *>(InitExp->getOp())->codegen(drv);
    if (!InitV)
      return nullptr;
  }
  else
  {
    // VARBINDING
    AllocaInst *boundval = std::get<BindingAST *>(InitExp->getOp())->codegen(drv);
    if (!boundval)
      return nullptr;
    tmpAlloca = drv.NamedValues[std::get<BindingAST *>(InitExp->getOp())->getName()];
    drv.NamedValues[std::get<BindingAST *>(InitExp->getOp())->getName()] = boundval;
  }

  // Creo i vari BB che serviranno e inserisco, nella funzione padre, quello per il controllo della condizione.
  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *CondBB = BasicBlock::Create(*context, "condstmt", function);
  BasicBlock *LoopBB = BasicBlock::Create(*context, "loopstmt");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "mergestmt");

  // Dal blocco in cui sono creo un salto incodizionato verso il blocco
  // che si occuperà del calcolo della condizione e setto il punto di inserimento.
  builder->CreateBr(CondBB);
  builder->SetInsertPoint(CondBB);

  // Generazione codice condizione per condizione.
  Value *condV = CondExpr->codegen(drv);
  if (!condV)
    return nullptr;

  // Dopo aver generato il codice creo un salto condizionato.
  // vero -> loop body
  // falso -> mergeBB (esci dal loop)
  builder->CreateCondBr(condV, LoopBB, MergeBB);

  // inserisco il codice per il controllo della condizione
  CondBB = builder->GetInsertBlock();
  function->insert(function->end(), LoopBB);

  // Inizio a scrivere il loop body
  // generazione del body del loop
  builder->SetInsertPoint(LoopBB);
  Value *loopV = BodyStmt->codegen(drv);
  if (!loopV)
    return nullptr;

  // generazione del codice per l'assignment
  Value *assignmentV = AssignExpr->codegen(drv);
  if (!assignmentV)
    return nullptr;

  // Salto incodizionato per il controllo della condizione
  builder->CreateBr(CondBB);

  // Inserisco il codice del Merge
  LoopBB = builder->GetInsertBlock();
  function->insert(function->end(), MergeBB);

  builder->SetInsertPoint(MergeBB);

  // Ripristino dello scope
  if (!InitExp->getOp().index())
    drv.NamedValues[std::get<BindingAST *>(InitExp->getOp())->getName()] = tmpAlloca;

  return Constant::getNullValue(Type::getDoubleTy(*context));
};

/************************* WhileStmtAST **************************/

WhileStmtAST::WhileStmtAST(ExprAST *CondExpr, StmtAST *BodyStmt) : CondExpr(CondExpr), BodyStmt(BodyStmt){};

Value *WhileStmtAST::codegen(driver &drv)
{
  // Creo i vari BB che serviranno e inserisco, nella funzione padre, quello per il controllo della condizione.
  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *CondBB = BasicBlock::Create(*context, "condstmt", function);
  BasicBlock *LoopBB = BasicBlock::Create(*context, "loopstmt");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "mergestmt");

  // Dal blocco in cui sono creo un salto incodizionato verso il blocco
  // che si occuperà del calcolo della condizione e setto il punto di inserimento.
  builder->CreateBr(CondBB);
  builder->SetInsertPoint(CondBB);

  // Generazione codice condizione per condizione.
  Value *condV = CondExpr->codegen(drv);
  if (!condV)
    return nullptr;

  // Dopo aver generato il codice creo un salto condizionato.
  // vero -> loop body
  // falso -> mergeBB (esci dal loop)
  builder->CreateCondBr(condV, LoopBB, MergeBB);

  // inserisco il codice per il controllo della condizione
  CondBB = builder->GetInsertBlock();
  function->insert(function->end(), LoopBB);

  // Inizio a scrivere il loop body
  // generazione del body del loop
  builder->SetInsertPoint(LoopBB);
  Value *loopV = BodyStmt->codegen(drv);
  if (!loopV)
    return nullptr;

  // Salto incodizionato per il controllo della condizione
  builder->CreateBr(CondBB);

  // Inserisco il codice del Merge
  LoopBB = builder->GetInsertBlock();
  function->insert(function->end(), MergeBB);

  builder->SetInsertPoint(MergeBB);
  return Constant::getNullValue(Type::getDoubleTy(*context));
};

/************************* DoWhileStmtAST **************************/

DoWhileStmtAST::DoWhileStmtAST(StmtAST *BodyStmt, ExprAST *CondExpr) : BodyStmt(BodyStmt), CondExpr(CondExpr){};

Value *DoWhileStmtAST::codegen(driver &drv)
{
  // Creo i vari BB che serviranno e inserisco, nella funzione padre, quello per il controllo della condizione.
  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *LoopBB = BasicBlock::Create(*context, "loopstmt", function);
  BasicBlock *CondBB = BasicBlock::Create(*context, "condstmt");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "mergestmt");

  // salto incodizionato verso il body
  builder->CreateBr(LoopBB);

  // Inizio a scrivere il loop body
  builder->SetInsertPoint(LoopBB);
  Value *loopV = BodyStmt->codegen(drv);
  if (!loopV)
    return nullptr;

  // Inserico il BB dedicato al controllo della condizione
  LoopBB = builder->GetInsertBlock();
  function->insert(function->end(), CondBB);

  // Dal blocco in cui sono creo un salto incodizionato verso il blocco
  // che si occuperà del calcolo della condizione e setto il punto di inserimento.
  builder->CreateBr(CondBB);

  // Generazione codice condizione per condizione.
  builder->SetInsertPoint(CondBB);
  Value *condV = CondExpr->codegen(drv);
  if (!condV)
    return nullptr;

  // Dopo aver generato il codice creo un salto condizionato.
  // vero -> loop body
  // falso -> mergeBB (esci dal loop)
  builder->CreateCondBr(condV, LoopBB, MergeBB);

  // Inserisco il codice del Merge
  CondBB = builder->GetInsertBlock();
  function->insert(function->end(), MergeBB);

  builder->SetInsertPoint(MergeBB);
  return Constant::getNullValue(Type::getDoubleTy(*context));
};

/*************************** ForEachStmtASt *********************+*/

ForEachStmtAST::ForEachStmtAST(const std::string IterName, const std::string ArrayName, StmtAST *BodyStmt) : IterName(IterName), ArrayName(ArrayName), BodyStmt(BodyStmt){};

std::string ForEachStmtAST::getCounterName() const {
  return IterName + "_counter";
}

Value *ForEachStmtAST::codegen(driver &drv)
{
  //Vengono creati tre BB dedicati al controllo della condizione, body del loop e fine del loop. 
  builder->SetInsertPoint(builder->GetInsertBlock());
  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *CondBB = BasicBlock::Create(*context, "condstmt", function);
  BasicBlock *LoopBB = BasicBlock::Create(*context, "loopstmt");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "mergestmt");
  
  // Nel raro caso in cui sia presente una altra variabile con lo stesso nome del Counter occorre
  // preservare lo scope. 
  AllocaInst *tmpCounter = drv.NamedValues[getCounterName()];
  //Si crea una nuova variabile contatore che terrà traccia dell'offset raggiunto volta per volta. 
  VarBindingAST *counter = new VarBindingAST(getCounterName(), new NumberExprAST(0.0));

  // Viene generato il boundVal del Counter e assegnamento nella Symbol Table. 
  AllocaInst *boundValCounter = counter->codegen(drv);
  if(!counter)
    return nullptr;
  drv.NamedValues[getCounterName()] = boundValCounter;

  //Si controlla che ArrayName faccia effettivamente riferimento ad un Array (già allocato)
  AllocaInst *A = drv.NamedValues[ArrayName];
  Type *AT;
  if (!A){
    GlobalVariable *A = module->getNamedGlobal(ArrayName);
    if (!A)
      return LogErrorV("Variabile " + ArrayName + " non definita.");
    AT = A->getValueType();
  } else{
    AT = A->getAllocatedType();
  }
  if (!AT->isArrayTy())
  {
    return LogErrorV("La variabile " + ArrayName + " non è un array.");
  }
  // La dimensione dell'array è necessaria per capire quando fermare il ciclo. 
  int ArraySize = AT->getArrayNumElements();

  // Alla fine del BB da cui stavo scrivendo prima creo un salto 
  //incodizionato verso il condBB e setto il punto di scrittura. 
  builder->CreateBr(CondBB);
  builder->SetInsertPoint(CondBB);
  
  // La condizione di salto sarà dettata da una espressione binaria in cui si controlla 
  // che il counter sia minore della dimensione dell'array.
  BinaryExprAST *CondExpr = new BinaryExprAST('<', new VariableExprAST(getCounterName()), new NumberExprAST(ArraySize));
  Value *condV = CondExpr->codegen(drv);
  if (!condV)
    return nullptr;
  
  // Se il counter è minore della dimensione si salta al LoopBB, altrimenti si esce. 
  builder->CreateCondBr(condV, LoopBB, MergeBB);

  //Si inserisce correttamente il LoopBB e si setta il punto di scrittura
  CondBB = builder->GetInsertBlock();
  function->insert(function->end(), LoopBB);
  builder->SetInsertPoint(LoopBB);

  // Si preserva lo scope nel caso in cui ci sia 
  // un'altra variabile con lo stesso nome dell'iteratore. 
  AllocaInst *tmpIter = drv.NamedValues[IterName];

  // Si crea un binding tra l'iteratore e Array[counter] 
  // ossia la cella dell'array a cui l'iteratore è arrivato.  
  VarBindingAST *var = new VarBindingAST(IterName, new ArrayExprAST(ArrayName, new VariableExprAST(getCounterName())));
  AllocaInst *bindingValue = var->codegen(drv);
  if (!bindingValue)
    return nullptr;
  drv.NamedValues[IterName] = bindingValue;

  // Si genera il codice del Body. 
  Value *BodyValue = BodyStmt->codegen(drv);
  if (!BodyValue)
    return nullptr;

  // Si incrementa il contatore. 
  AssignmentAST *updateCounter = new AssignmentAST(getCounterName(), new BinaryExprAST('+', new VariableExprAST(getCounterName()), new NumberExprAST(1.0)));
  updateCounter->codegen(drv);
  if (!updateCounter)
    return nullptr;

  //Si crea un salto incodizionato verso CondBB
  LoopBB = builder->GetInsertBlock();
  builder->CreateBr(CondBB);

  // Viene inserito alla fine il MergeBB. 
  function->insert(function->end(), MergeBB);
  builder->SetInsertPoint(MergeBB);
  
  // Ripristinato lo scope. 
  drv.NamedValues[getCounterName()] = tmpCounter;
  drv.NamedValues[IterName] = tmpIter; 

  // Si ritorna 0. 
  return Constant::getNullValue(Type::getDoubleTy(*context));
};