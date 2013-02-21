__kernel void vector_add(__global const int *numberInput, __global int *c) {
    
    // Get the index to process.
    int i = get_global_id(0);
    
    // Do the operation.
    int number = numberInput[i];
    c[i] = -1;
    
    if (number & 1) {
        
        number = 3 * number + 1;
        c[i] ++;
    }
    
    while (number != 4) {
        
        do {
            
            number >>= 1;
            c[i]++;
        } while (!(number & 1));
        
        number = 3 * number + 1;
        c[i]++;
    }
    
    //c[i] = number;
}
