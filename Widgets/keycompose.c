#include "keycompose.h"

char ComposeKeyEn_ToRu(char key, char shift){
    switch(key){
    case 'q': if(shift) return '�'; else return '�';
    case 'w': if(shift) return '�'; else return '�';
    case 'e': if(shift) return '�'; else return '�';
    case 'r': if(shift) return '�'; else return '�';
    case 't': if(shift) return '�'; else return '�';
    case 'y': if(shift) return '�'; else return '�';
    case 'u': if(shift) return '�'; else return '�';
    case 'i': if(shift) return '�'; else return '�';
    case 'o': if(shift) return '�'; else return '�';
    case 'p': if(shift) return '�'; else return '�';
    case '[': if(shift) return '�'; else return '�';
    case ']': if(shift) return '�'; else return '�';
    case 'a': if(shift) return '�'; else return '�';
    case 's': if(shift) return '�'; else return '�';
    case 'd': if(shift) return '�'; else return '�';
    case 'f': if(shift) return '�'; else return '�';
    case 'g': if(shift) return '�'; else return '�';
    case 'h': if(shift) return '�'; else return '�';
    case 'j': if(shift) return '�'; else return '�';
    case 'k': if(shift) return '�'; else return '�';
    case 'l': if(shift) return '�'; else return '�';
    case ';': if(shift) return '�'; else return '�';
    case '\'': if(shift) return '�'; else return '�';
    case 'z': if(shift) return '�'; else return '�';
    case 'x': if(shift) return '�'; else return '�';
    case 'c': if(shift) return '�'; else return '�';
    case 'v': if(shift) return '�'; else return '�';
    case 'b': if(shift) return '�'; else return '�';
    case 'n': if(shift) return '�'; else return '�';
    case 'm': if(shift) return '�'; else return '�';
    case ',': if(shift) return '�'; else return '�';
    case '.': if(shift) return '�'; else return '�';
    case '2': if(shift) return '"'; else return key;
    case '/': if(shift) return ','; else return '.';
    case '1': if(shift) return '!'; else return key;
    case '`': if(shift) return '�'; else return '�';
    default: return key;
    }
}
