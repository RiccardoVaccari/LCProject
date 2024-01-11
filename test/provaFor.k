extern printval(x);
def provaFor(n) {
	var i = 0;
	do{
		printval(i);
		++i
	}while(i<n);

	while(i>0){
		printval(i);
		--i
	}
};