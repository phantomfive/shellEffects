//This is only used in shellEngine.c, but it is different enough it's worth
//putting in a separate file

//----------------------------------------------------------------------------
// Some list functions. Don't pass NULL into these. It's called crazylist
// because it annoys me that I have to do it at all. In 2011 there should
// be a generic list implementation in the C language.
//----------------------------------------------------------------------------
#define MAX_EFFECTS 20
struct List_t{
	ShellEffect *list[MAX_EFFECTS];
	int size;
};

static char listInit(List *list) {
	list->size = 0;
	return SUCCESS;
}

//returns true on success, false on failure
static char pushBack(List *list, ShellEffect *effect) {
	if(list->size>=MAX_EFFECTS) return 0;
	list->list[list->size] = effect;
	list->size++;
	return 1;
}

//returns NULL if nothing there
static ShellEffect *popBack(List *list) {
	ShellEffect *rv;
	if(list->size<=0) return NULL;
	list->size--;
	rv = list->list[list->size];
	return rv;
}

/*static void printList(List *list)
{
	int i;
	fprintf(stderr,"--------------------------------------\n");
	for(i=0;i<list->size;i++)
	{
		fprintf(stderr,"%d: %p\n", i, list->list[i]);
	}
	fprintf(stderr,"-------------------------------------\n");
}
*/

//returns the element passed in and removes them
static ShellEffect *removeElement(List *list, ShellEffect *toRemove) {
	ShellEffect *rv = toRemove;
	int i;
	int j;
	for(i=0;i<list->size;i++) {
		if(list->list[i]==toRemove) {
			//found one, move everything down. looks inefficient but
			//it's not likely we'll have more than 10 elements so it's really not
			for(j=i;j<list->size-1;j++){
				list->list[j] = list->list[j+1];
			}
			list->size--;
		}
	}
	return rv;
}

//basically a foreach loop for Lists
#define listForeach( listElement, list_ )                          \
Effect *listElement;                                               \
int i_             ;                                               \
for (i_=0; (i_<list_.size)&&(listElement = (Effect*)list_.list[i_]) ;i_++)

//lets you safely remove an element while in the previously mentioned
//foreach loop. Doesn't free it, so you better free it before calling this
#define removeInForeach(listElement, list_)         \
	{                                                \
		int i_2;                                      \
		for(i_2=i_;i_2<list_.size-1;i_2++)            \
			list_.list[i_2] = list_.list[i_2+1];       \
		i_--;                                         \
		list_.size--;                                 \
	}

