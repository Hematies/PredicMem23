# PredicMem23

El proyecto contiene 3 cabeceras donde se declaran las clases y funciones a usar:

### TraceReader.h (por hacer)

Declara las clases:

#### 1. TraceReader

Clase que se construye recibiendo un nombre de fichero de traza (por ejemplo, "pinatrace_lbm.out").
Implementará un método el cual recibe por entrada el número de accesos a leer N y devolverá:
- Un objeto tipo vector<long> con las siguientes N direcciones accedidas.
- Un objeto tipo vector<long> con los respectivos Program Counters (PCs) de las siguientes N instrucciones de acceso.
  
Las líneas de las trazas de acceso siguen el siguiente formato:
  <PC (hex)>: <W para escritura ó R para lectura> <direccón accedida (hex)> 
Por ejemplo:
    0x7f2974d88093: W 0x7ffeedfc8e88
    
    
### BufferSimulator.h

Declara las clases, interfaces y estructuras:

#### 1. AccessesDataset
    
Estructura con dos campos: 
- Una colección (vector<A>) de accesos, que en nuestro caso serán los direcciones accedidas por las instrucciones.
    
#### 2. BuffersDataset
    
#### 3. HistoryCache
    
#### 4. HistoryCacheEntry
    
#### 5. Dictionary
  
#### 6. DictionaryEntry
    
#### 7. BuffersSimulator
  
    
### PredictorSVM.h
    
### SVMClassifier.h
