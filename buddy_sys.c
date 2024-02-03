// Compilar: gcc buddy_sys.c -o buddy_sys
// Se soportan memorias disponibles no potencia de 2!

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Definicion propia booleans
#define bool int
#define true 1
#define false 0

// Constantes
#define error -1
#define idNoAsignado -1

// Structs
struct nodoBuddy
{
	int idProceso;
	int tamanio;
	int direccionInicial;
	struct nodoBuddy* hijoIzquierdo;
	struct nodoBuddy* hijoDerecho;
};

struct nodoParticion
{
	int direccionInicial;
	struct nodoParticion* siguiente;
};

// Variables globales
struct nodoBuddy* nodoRaiz;
int memoriaDisponible;
int contadorIds = 0;


// Procedimientos

// Ejercicio 1
// "Interfaz grafica"
void mostrarMenu();
void opcionNew();
void opcionFree();
void opcionList();
void opcionMap();

// Gestion arbol
struct nodoBuddy* crearNodoBuddy(int tamanio, int direccionInicial);
bool fueAsignado(struct nodoBuddy* nodo);
bool fueDividido(struct nodoBuddy* nodo);
int calcularCotaInferior(int base, int valor);

// Gestion lista particiones
struct nodoParticion* crearNodoParticion(int direccionInicial);
void anexarNodoParticion(struct nodoParticion** lista, struct nodoParticion* nodo);
void eliminarLista(struct nodoParticion** nodo);
void mostrarContenidoLista(struct nodoParticion* nodo);

// Ejercicio 2
int asignarMemoria(int tamanioProceso, struct nodoBuddy* nodo);

// Ejercicio 3
bool quitarProceso(int idProceso, struct nodoBuddy* nodo);
bool unirParticiones(struct nodoBuddy* nodo);

// Ejercicio 4
void listarParticiones(struct nodoBuddy* nodoBuddy, struct nodoParticion** listaParticion, int tamanioParticion);
int obtenerTamanioMinimo(struct nodoBuddy* nodo);

// Ejercicio 5
void mapearMemoria(struct nodoBuddy* nodo);



int main (int argc, char* argv[])
{
	char ch;	
	
	opterr = 0;   // no mostrar mensaje de error estandar
	ch = getopt(argc, argv, "s:");   
	if (ch == 's')
	{
		memoriaDisponible = atoi(optarg);		
		if (memoriaDisponible > 0)
			{
				nodoRaiz = crearNodoBuddy(memoriaDisponible, 0);
				mostrarMenu();
			}
		else
			printf("Error. La memoria disponible no puede ser negativa.\n");
	}		
	else
		printf("Error. Parametro incorrecto o memoria disponible no ingresada.\n");
	
	return 0;
}

// Ejercicio 1
// "Interfaz grafica"
void mostrarMenu()
{
	char str[16];	
	
	printf("B U D D Y  S Y S T E M\n\n");
	printf("[(N)EW]\n");
	printf("[(F)REE]\n");
	printf("[(L)IST]\n");
	printf("[(M)AP]\n");
	printf("[(E)XIT]\n");

	do
	{
		printf("\nOpcion: ");
		scanf("%s", &str);
	
		switch(str[0])
		{
			case 'N':
			case 'n':
				opcionNew();
				break;
			case 'F':
			case 'f':
				opcionFree();
				break;
			case 'L':
			case 'l':
				opcionList();
				break;
			case 'M':
			case 'm':
				opcionMap();
				break;
			case 'E':
			case 'e':
				// no hacer nada
				break;
			default:
				printf("Error. Opcion invalida\n");
		}
	}
	while(str[0] != 'e' && str[0] != 'E');   // solo se considera el primer caracter
}

// Ejercicio 2
void opcionNew()
{
	int tamanioProceso, idProceso;

	printf("MEMORY ALLOCATION - process size: ");
	scanf("%i", &tamanioProceso);
	
	idProceso = asignarMemoria(tamanioProceso, nodoRaiz);
	if (idProceso != error)
		printf("Resultado: asignacion exitosa. MID: %i\n", idProceso);
	else
		printf("Resultado: asignacion fallida.\n");
}

// Ejercicio 3
void opcionFree()
{
	int idProceso;
	bool existiaProceso;

	printf("MEMORY DEALLOCATION - process MID: ");
	scanf("%i", &idProceso);				
	
	existiaProceso = quitarProceso(idProceso, nodoRaiz);
	if (existiaProceso)
	{
		unirParticiones(nodoRaiz);
		printf("Resultado: liberacion exitosa.\n");
	}
	else
		printf("Error. MID invalido.\n");
}
// Ejercicio 4
void opcionList()
{
	struct nodoParticion* listaTamanio;
	struct nodoParticion* aux;
	int tamanio, tamanioMinimo;

	printf("Tamanio\t\tLista direcciones base de particiones\n");

	if (!fueDividido(nodoRaiz))
		tamanio = memoriaDisponible;
	else
		tamanio = calcularCotaInferior(1, memoriaDisponible);


	tamanioMinimo = obtenerTamanioMinimo(nodoRaiz);
	listaTamanio = NULL;
	while (tamanio >= tamanioMinimo)
	{		
		listarParticiones(nodoRaiz, &listaTamanio, tamanio);
		
		if (listaTamanio != NULL)
		{
			printf("%i K\t\t", tamanio);
			mostrarContenidoLista(listaTamanio);
		}	

		eliminarLista(&listaTamanio);	
		tamanio /= 2;
	}	
}

// Ejercicio 5
void opcionMap()
{
	printf("Direccion base\tTamanio\t\tMID\n");
	mapearMemoria(nodoRaiz);	
}


// Gestion arbol
struct nodoBuddy* crearNodoBuddy(int tamanio, int direccionInicial)
{
	struct nodoBuddy* nuevoNodo;

	nuevoNodo = (struct nodoBuddy*) malloc(sizeof(struct nodoBuddy));
	
	nuevoNodo -> idProceso = idNoAsignado;
	nuevoNodo -> tamanio = tamanio;
	nuevoNodo -> direccionInicial = direccionInicial;
	nuevoNodo -> hijoIzquierdo = NULL;
	nuevoNodo -> hijoDerecho = NULL;

	return nuevoNodo;	
}

bool fueAsignado(struct nodoBuddy* nodo)
{
	return (nodo -> idProceso != idNoAsignado);
}

bool fueDividido(struct nodoBuddy* nodo)
{
	return (nodo -> hijoIzquierdo != NULL && nodo -> hijoDerecho != NULL);
}

int calcularCotaInferior(int base, int valor) 
{
	/*
	Contempla el caso de memoria disponible no potencia de 2. Llamar con base = 1.
	Equivalente a:
		(int) pow(2, ceil(log2(valor)) - 1.0); 
	*/
	if (valor == 1)
		return 0;
	else	
		if (2 * base >= valor)
			return base;
		else
			return calcularCotaInferior(2 * base, valor);
}



// Gestion lista particiones
struct nodoParticion* crearNodoParticion(int direccionInicial)
{
	struct nodoParticion* nuevoNodo;

	nuevoNodo = (struct nodoParticion*) malloc(sizeof(struct nodoParticion));

	nuevoNodo -> direccionInicial = direccionInicial;
	nuevoNodo -> siguiente = NULL;	

	return nuevoNodo;
}

void anexarNodoParticion(struct nodoParticion** lista, struct nodoParticion* nodo)
{
	if (*lista == NULL)
		*lista = nodo;
	else 
		anexarNodoParticion(&((*lista) -> siguiente), nodo);
}

void eliminarLista(struct nodoParticion** nodo)
{
	if (*nodo != NULL)
	{
		eliminarLista(&((*nodo) -> siguiente));
		free(*nodo);
		*nodo = NULL;
	}
}

void mostrarContenidoLista(struct nodoParticion* nodo)
{
	if (nodo == NULL)
		printf("NULL\n");
	else
	{
		printf("%i -> ", nodo -> direccionInicial);
		mostrarContenidoLista(nodo -> siguiente);
	}				
}


// Ejercicio 2
int asignarMemoria(int tamanioProceso, struct nodoBuddy* nodo)
{
	int cotaInferior, cotaSuperior, idProcesoAsignado;

	cotaSuperior = nodo -> tamanio;
	cotaInferior = calcularCotaInferior(1, nodo -> tamanio);
 
	if (tamanioProceso <= cotaSuperior && tamanioProceso > cotaInferior)
		if (!fueAsignado(nodo) && !fueDividido(nodo))		
		{			
			contadorIds++;
			nodo -> idProceso = contadorIds;
			return contadorIds;
		}		
		else
			return error; 
	else if (tamanioProceso > 0 && tamanioProceso <= cotaInferior)
		if (fueAsignado(nodo))
			return error;
		else
		{
			if (!fueDividido(nodo))
			{
				nodo -> hijoIzquierdo = crearNodoBuddy(cotaInferior, nodo -> direccionInicial);
				nodo -> hijoDerecho = crearNodoBuddy(cotaSuperior - cotaInferior, nodo -> direccionInicial + cotaInferior);
			}

			idProcesoAsignado = asignarMemoria(tamanioProceso, nodo -> hijoIzquierdo);
			if (idProcesoAsignado == error)   // ver en el hijo derecho solo si no se pudo asignar en el izquierdo
				return asignarMemoria(tamanioProceso, nodo -> hijoDerecho);
			else
				return idProcesoAsignado;		
		}
	else  
		return error;		
}


// Ejercicio 3
bool quitarProceso(int idProceso, struct nodoBuddy* nodo)
{
	if (!fueDividido(nodo))
		if (nodo -> idProceso == idProceso)
		{
			nodo -> idProceso = idNoAsignado;
			return true;
		}
		else 
			return false;
	else
		return 
			(quitarProceso(idProceso, nodo -> hijoIzquierdo) || 
			 quitarProceso(idProceso, nodo -> hijoDerecho));
}

bool unirParticiones(struct nodoBuddy* nodo) 
{
	bool hijoIzquierdoLibre, hijoDerechoLibre;

	if (!fueDividido(nodo))
		if (!fueAsignado(nodo))
			return true;
		else
			return false;
	else 
	{	
		hijoIzquierdoLibre = unirParticiones(nodo -> hijoIzquierdo);
		hijoDerechoLibre = unirParticiones(nodo -> hijoDerecho);	

		if (hijoIzquierdoLibre && hijoDerechoLibre)   // colocar los unirParticiones() con && hace que a veces no se evalue para el hijo derecho
		{
			free(nodo -> hijoIzquierdo);   // liberar memoria
			nodo -> hijoIzquierdo = NULL;

			free(nodo -> hijoDerecho);
			nodo -> hijoDerecho = NULL;

			return true;
		}
		else
			return false;
	}
}


// Ejercicio 4
void listarParticiones(struct nodoBuddy* nodoBuddy, struct nodoParticion** listaParticion, int tamanioParticion)
{
	if (!fueDividido(nodoBuddy))	
	{
		if (nodoBuddy -> tamanio == tamanioParticion)
			anexarNodoParticion(listaParticion, crearNodoParticion(nodoBuddy -> direccionInicial));
	}
	else
	{
		listarParticiones(nodoBuddy -> hijoIzquierdo, listaParticion, tamanioParticion);
		listarParticiones(nodoBuddy -> hijoDerecho, listaParticion, tamanioParticion);
	}				
}

int obtenerTamanioMinimo(struct nodoBuddy* nodo)
{
	int tamanioHijoIzquierdo, tamanioHijoDerecho;	

	if (!fueDividido(nodo))
		return (nodo -> tamanio);
	else
	{
		tamanioHijoIzquierdo = obtenerTamanioMinimo(nodo -> hijoIzquierdo);
		tamanioHijoDerecho = obtenerTamanioMinimo(nodo -> hijoDerecho);

		if (tamanioHijoIzquierdo < tamanioHijoDerecho)
			return tamanioHijoIzquierdo;
		else
			return tamanioHijoDerecho;
	} 
}


// Ejercicio 5
void mapearMemoria(struct nodoBuddy* nodo)
{
	if (!fueDividido(nodo))
		if (!fueAsignado(nodo))
			printf("%i\t\t%i\t\tLibre\n", nodo -> direccionInicial, nodo -> tamanio);
		else
			printf("%i\t\t%i\t\t%i\n", nodo -> direccionInicial, nodo -> tamanio, nodo -> idProceso);
	else
	{
		mapearMemoria(nodo -> hijoIzquierdo);
		mapearMemoria(nodo -> hijoDerecho);
	}
}

