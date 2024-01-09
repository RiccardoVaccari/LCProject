
extern printval(x);
global V[5];

def provaArray(y) {
	var size = 10;
	var V[10];
	var i = 0;
	
	for(var i = 0; i < size;++i){
		V[i] = y + i
	};

	while(i < size){
		printval(V[i]);
		++i
	};
	0
};

def provaArray2(){
	V[6] = 1;
	for(var i = 0; i < 7; ++i){
		printval(V[i])
	};
	0
};