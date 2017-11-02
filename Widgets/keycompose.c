#include "keycompose.h"

char ComposeKeyEn_ToRu(char key, char shift){
    switch(key){
    case 'q': if(shift) return 'ê'; else return 'Ê';
    case 'w': if(shift) return 'ã'; else return 'Ã';
    case 'e': if(shift) return 'õ'; else return 'Õ';
    case 'r': if(shift) return 'ë'; else return 'Ë';
    case 't': if(shift) return 'å'; else return 'Å';
    case 'y': if(shift) return 'î'; else return 'Î';
    case 'u': if(shift) return 'ç'; else return 'Ç';
    case 'i': if(shift) return 'û'; else return 'Û';
    case 'o': if(shift) return 'ı'; else return 'İ';
    case 'p': if(shift) return 'ú'; else return 'Ú';
    case '[': if(shift) return 'è'; else return 'È';
    case ']': if(shift) return 'ÿ'; else return 'ß';
    case 'a': if(shift) return 'æ'; else return 'Æ';
    case 's': if(shift) return 'ù'; else return 'Ù';
    case 'd': if(shift) return '÷'; else return '×';
    case 'f': if(shift) return 'á'; else return 'Á';
    case 'g': if(shift) return 'ğ'; else return 'Ğ';
    case 'h': if(shift) return 'ò'; else return 'Ò';
    case 'j': if(shift) return 'ï'; else return 'Ï';
    case 'k': if(shift) return 'ì'; else return 'Ì';
    case 'l': if(shift) return 'ä'; else return 'Ä';
    case ';': if(shift) return 'ö'; else return 'Ö';
    case '\'': if(shift) return 'ü'; else return 'Ü';
    case 'z': if(shift) return 'ñ'; else return 'Ñ';
    case 'x': if(shift) return 'ş'; else return 'Ş';
    case 'c': if(shift) return 'ó'; else return 'Ó';
    case 'v': if(shift) return 'í'; else return 'Í';
    case 'b': if(shift) return 'é'; else return 'É';
    case 'n': if(shift) return 'ô'; else return 'Ô';
    case 'm': if(shift) return 'ø'; else return 'Ø';
    case ',': if(shift) return 'â'; else return 'Â';
    case '.': if(shift) return 'à'; else return 'À';
    case '2': if(shift) return '"'; else return key;
    case '/': if(shift) return ','; else return '.';
    case '1': if(shift) return '!'; else return key;
    case '`': if(shift) return '³'; else return '£';
    default: return key;
    }
}
