//
// Created by kdehairy on 11/11/15.
//

#include <DexUtf.h>
#include <malloc.h>
#include "neutralize.h"
#include "hashmap/hashmap.h"
#include "DexClass.h"

static map_t stringIdsMap;
static map_t typeIdsMap;
static map_t protoIdsMap;

static map_t fieldIdsMap;
static map_t methodIdsMap;

static void cleanup() {
    size_t total_size = 0;
    if ( stringIdsMap ) {
        size_t size = hashmap_size( stringIdsMap );
        total_size += size;
        printf( "stingsMap: %zu [%zu]\n", size, size * 2 * sizeof( uint32_t ));
        hashmap_free( stringIdsMap );
    }

    if ( typeIdsMap ) {
        size_t size = hashmap_size( typeIdsMap );
        total_size += size;
        printf( "typeIdMap: %zu [%zu]\n", size, size * 2 * sizeof( uint32_t ));
        hashmap_free( typeIdsMap );
    }

    if ( protoIdsMap ) {
        size_t size = hashmap_size( protoIdsMap );
        total_size += size;
        printf( "protoIdMap: %zu [%zu]\n", size, size * 2 * sizeof( uint32_t ));
        hashmap_free( protoIdsMap );
    }

    if ( fieldIdsMap ) {
        size_t size = hashmap_size( fieldIdsMap );
        total_size += size;
        printf( "fieldIdMap: %zu [%zu]\n", size, size * 2 * sizeof( uint32_t ));
        hashmap_free( fieldIdsMap );
    }

    if ( methodIdsMap ) {
        size_t size = hashmap_size( methodIdsMap );
        total_size += size;
        printf( "methodIdMap: %zu [%zu]\n", size,
                size * 2 * sizeof( uint32_t ));
        hashmap_free( methodIdsMap );
    }

    printf( "total overhead: %zu [%zu]\n", total_size,
            total_size * 2 * sizeof( uint32_t ));
}

static void mapStringIds( const DexFile *pBaseDex,
        const DexFile *pVariantDex ) {
    uint32_t baseStringIdsSize = pBaseDex->pHeader->stringIdsSize;
    uint32_t variantStringIdsSize = pVariantDex->pHeader->stringIdsSize;

    stringIdsMap = hashmap_new();

    uint32_t i = 0;
    uint32_t j = 0;

    while ( i < baseStringIdsSize && j < variantStringIdsSize ) {
        const char *pBaseStringData = dexStringById( pBaseDex, i );
        const char *pVariantStringData = dexStringById( pVariantDex, j );
        int cmp = dexUtf8Cmp( pBaseStringData, pVariantStringData );
        if ( cmp < 0 ) {
            i++;
        } else if ( cmp > 0 ) {
            j++;
        } else {

            if ( i != j ) {
                hashmap_put( stringIdsMap, i, j );
#ifdef KDEBUG
                fprintf( stdout, "[%d] == [%d]\n", i, j );
#endif
            }
            i++;
            j++;
        }
    }
}

static void neutralizeTypeId( DexTypeId *pTypeId ) {
    uint32_t newIdx;
    if ( hashmap_get( stringIdsMap, pTypeId->descriptorIdx, &newIdx ) ==
         MAP_OK ) {
        pTypeId->descriptorIdx = newIdx;
    }
}

static void mapTypeIds( const DexFile *pBaseDex,
        const DexFile *pVariantDex ) {
    uint32_t baseTypeIdsSize = pBaseDex->pHeader->typeIdsSize;
    uint32_t variantTypeIdsSize = pVariantDex->pHeader->typeIdsSize;

    typeIdsMap = hashmap_new();

    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t lastNeutralized = 0;
    bool isNeutralized = false;

    while ( i < baseTypeIdsSize && j < variantTypeIdsSize ) {
        DexTypeId *pBaseTypeId = dexGetTypeId( pBaseDex, i );
        if ( !isNeutralized ) {
            neutralizeTypeId( pBaseTypeId );
            isNeutralized = true;
            lastNeutralized = i;
        }

        const DexTypeId *pVariantTypeId = dexGetTypeId( pVariantDex, j );
        int cmp = pBaseTypeId->descriptorIdx - pVariantTypeId->descriptorIdx;
        if ( cmp < 0 ) {
            i++;
            isNeutralized = false;
        } else if ( cmp > 0 ) {
            j++;
        } else {

            if ( i != j ) {
                hashmap_put( typeIdsMap, i, j );
#ifdef KDEBUG
                fprintf( stdout, "[%d] %d == [%d] %d\n", i,
                        pBaseTypeId->descriptorIdx, j,
                        pVariantTypeId->descriptorIdx );
#endif
            }
            i++;
            isNeutralized = false;
            j++;
        }
    }

    i = lastNeutralized + 1;
    while ( i < baseTypeIdsSize ) {
        DexTypeId *pBaseTypeId = dexGetTypeId( pBaseDex, i );
        neutralizeTypeId( pBaseTypeId );
        i++;
    }
}

static int compareFieldIds( const DexFieldId *pFieldId_1,
        const DexFieldId *pFieldId_2 ) {
    uint32_t classIdx_1 = pFieldId_1->classIdx;
    uint32_t classIdx_2 = pFieldId_2->classIdx;

    int cmp = classIdx_1 - classIdx_2;
    if ( cmp != 0 ) {
        return cmp;
    }

    uint32_t nameIdx_1 = pFieldId_1->nameIdx;
    uint32_t nameIdx_2 = pFieldId_2->nameIdx;

    cmp = nameIdx_1 - nameIdx_2;
    if ( cmp != 0 ) {
        return cmp;
    }

    uint32_t typeIdx_1 = pFieldId_1->typeIdx;
    uint32_t typeIdx_2 = pFieldId_2->typeIdx;

    cmp = typeIdx_1 - typeIdx_2;
    if ( cmp != 0 ) {
        return cmp;
    }

    return 0;
}

static int compareMethodIds( DexMethodId *pBaseId,
        const DexMethodId *pVariantId ) {

    int cmp = pBaseId->classIdx - pVariantId->classIdx;
    if ( cmp != 0 ) {
        return cmp;
    }

    cmp = pBaseId->nameIdx - pVariantId->nameIdx;
    if ( cmp != 0 ) {
        return cmp;
    }

    cmp = pBaseId->protoIdx - pVariantId->protoIdx;
    if ( cmp != 0 ) {
        return cmp;
    }

    return 0;
}

static int compareProtoIds( const DexFile *pDex_1, const DexProtoId *pProtoId_1,
        const DexFile *pDex_2,
        const DexProtoId *pProtoId_2 ) {
    uint32_t returnType_1 = pProtoId_1->returnTypeIdx;
    uint32_t returnType_2 = pProtoId_2->returnTypeIdx;

    int cmp = returnType_1 - returnType_2;
    if ( cmp != 0 ) {
        return cmp;
    }

    DexTypeList *pParams_1 = dexGetProtoParameters( pDex_1, pProtoId_1 );
    DexTypeList *pParams_2 = dexGetProtoParameters( pDex_2, pProtoId_2 );

    uint32_t size_1 = 0;
    uint32_t size_2 = 0;
    if ( pParams_1 ) {
        size_1 = pParams_1->size;
    }
    if ( pParams_2 ) {
        size_2 = pParams_2->size;
    }

    cmp = size_1 - size_2;
    if ( cmp != 0 ) {
        return cmp;
    }

    if ( size_1 == 0 || size_2 == 0 ) {
        return cmp;
    }

    for ( uint32_t i = 0; i < pParams_1->size; ++i ) {
        DexTypeItem item_1 = pParams_1->list[i];
        DexTypeItem item_2 = pParams_2->list[i];
        cmp = item_1.typeIdx - item_2.typeIdx;
        if ( cmp != 0 ) {
            return cmp;
        }
    }
    return 0;
}

static void neutralizeProtoId( const DexFile *pDex, DexProtoId *pProtoId ) {
    // neutralize shortIdx
    uint32_t newIdx;
    if ( hashmap_get( stringIdsMap, pProtoId->shortyIdx, &newIdx ) == MAP_OK ) {
        pProtoId->shortyIdx = newIdx;
    }

    //neutralize return type idx
    if ( hashmap_get( typeIdsMap, pProtoId->returnTypeIdx, &newIdx ) ==
         MAP_OK ) {
        pProtoId->returnTypeIdx = newIdx;
    }

    //neutralize parameters type idx
    DexTypeList *paramList = dexGetProtoParameters( pDex, pProtoId );

    if ( paramList ) {
        for ( uint32_t j = 0; j < paramList->size; ++j ) {
            if ( hashmap_get( typeIdsMap, paramList->list[j].typeIdx,
                    &newIdx ) == MAP_OK ) {
                paramList->list[j].typeIdx = (uint16_t) newIdx;
            }
        }
    }
}

static void mapProtoIds( const DexFile *pBaseDex,
        const DexFile *pVariantDex ) {
    uint32_t baseProtoIdsSize = pBaseDex->pHeader->protoIdsSize;
    uint32_t variantProtoIdsSize = pVariantDex->pHeader->protoIdsSize;

    protoIdsMap = hashmap_new();

    uint32_t i = 0;
    uint32_t j = 0;

    uint32_t lastNeutralized = 0;
    bool isNeutralized = false;

    while ( i < baseProtoIdsSize && j < variantProtoIdsSize ) {
        DexProtoId *pBaseProtoId = dexGetProtoId( pBaseDex, i );
        if ( !isNeutralized ) {
            neutralizeProtoId( pBaseDex, pBaseProtoId );
            isNeutralized = true;
            lastNeutralized = i;
        }

        const DexProtoId *pVariantProtoId = dexGetProtoId( pVariantDex, j );
        int cmp = compareProtoIds( pBaseDex, pBaseProtoId, pVariantDex,
                pVariantProtoId );
        if ( cmp < 0 ) {
            i++;
            isNeutralized = false;
        } else if ( cmp > 0 ) {
            j++;
        } else {
            if ( i != j ) {
                hashmap_put( protoIdsMap, i, j );
#ifdef KDEBUG
                fprintf( stdout, "%d == %d\n", i, j );
#endif
            }
            i++;
            isNeutralized = false;
            j++;
        }
    }

    i = lastNeutralized + 1;

    while ( i < baseProtoIdsSize ) {
        DexProtoId *pBaseProtoId = dexGetProtoId( pBaseDex, i );
        neutralizeProtoId( pBaseDex, pBaseProtoId );
        i++;
    }
}

static void neutralizeFieldId( DexFieldId *pFieldId ) {
    // neutralize typeIdx
    uint32_t newTypeIdx;
    if ( hashmap_get( typeIdsMap, pFieldId->typeIdx,
            &newTypeIdx ) == MAP_OK ) {
        pFieldId->typeIdx = (uint16_t) newTypeIdx;
    }

    // neutralize nameIdx
    uint32_t newNameIdx;
    if ( hashmap_get( stringIdsMap, pFieldId->nameIdx,
            &newNameIdx ) == MAP_OK ) {
        pFieldId->nameIdx = newNameIdx;
    }

    // neutralize classIdx
    uint32_t newClassIdx;
    if ( hashmap_get( typeIdsMap, pFieldId->classIdx,
            &newClassIdx ) == MAP_OK ) {
        pFieldId->classIdx = (uint16_t) newClassIdx;
    }
}

static void neutralizeMethodId( DexMethodId *pMethodId ) {
    uint32_t newIdx;
    if ( hashmap_get( typeIdsMap, pMethodId->classIdx, &newIdx ) == MAP_OK ) {
        pMethodId->classIdx = (uint16_t) newIdx;
    }

    if ( hashmap_get( stringIdsMap, pMethodId->nameIdx, &newIdx ) == MAP_OK ) {
        pMethodId->nameIdx = newIdx;
    }

    if ( hashmap_get( protoIdsMap, pMethodId->protoIdx, &newIdx ) == MAP_OK ) {
        pMethodId->protoIdx = (uint16_t) newIdx;
    }
}


static void neutralizeMethodIds( DexFile *pBaseDex,
        const DexFile *pVariantDex ) {

    uint32_t baseSize = pBaseDex->pHeader->methodIdsSize;
    uint32_t variantSize = pVariantDex->pHeader->methodIdsSize;

    methodIdsMap = hashmap_new();

    uint32_t i = 0;
    uint32_t j = 0;

    uint32_t lastNeutralized = 0;
    bool isNeutralized = false;

    while ( i < baseSize && j < variantSize ) {
        DexMethodId *pBaseMethodId = dexGetMethodId( pBaseDex, i );
        if ( !isNeutralized ) {
            neutralizeMethodId( pBaseMethodId );
            isNeutralized = true;
            lastNeutralized = i;
        }

        const DexMethodId *pVariantMethodId = dexGetMethodId( pVariantDex, j );
        int cmp = compareMethodIds( pBaseMethodId, pVariantMethodId );
        if ( cmp < 0 ) {
            i++;
            isNeutralized = false;
        } else if ( cmp > 0 ) {
            j++;
        } else {
            if ( i != j ) {
                hashmap_put( methodIdsMap, i, j );
#ifdef KDEBUG
                fprintf( stdout, "%d == %d\n", i, j );
#endif
            }
            i++;
            isNeutralized = false;
            j++;
        }
    }
    i = lastNeutralized + 1;

    while ( i < baseSize ) {
        DexMethodId *pBaseMethodId = dexGetMethodId( pBaseDex, i );
        neutralizeMethodId( pBaseMethodId );
        i++;
    }
}

static void mapFieldIds( const DexFile *pBaseDex, const DexFile *pVariantDex ) {
    uint32_t baseFieldIdsSize = pBaseDex->pHeader->fieldIdsSize;
    uint32_t variantFieldIdsSize = pVariantDex->pHeader->fieldIdsSize;

    fieldIdsMap = hashmap_new();

    uint32_t i = 0;
    uint32_t j = 0;

    uint32_t lastNeutralized = 0;
    bool isNeutralized = false;

    while ( i < baseFieldIdsSize && j < variantFieldIdsSize ) {
        DexFieldId *pBaseFieldId = dexGetFieldId( pBaseDex, i );
        if ( !isNeutralized ) {
            neutralizeFieldId( pBaseFieldId );
            isNeutralized = true;
            lastNeutralized = i;
        }

        const DexFieldId *pVariantFieldId = dexGetFieldId( pVariantDex, j );
        int cmp = compareFieldIds( pBaseFieldId, pVariantFieldId );
        if ( cmp < 0 ) {
            i++;
            isNeutralized = false;
        } else if ( cmp > 0 ) {
            j++;
        } else {
            if ( i != j ) {
                hashmap_put( fieldIdsMap, i, j );
#ifdef KDEBUG
                fprintf( stdout, "%d == %d\n", i, j );
#endif
            }
            i++;
            isNeutralized = false;
            j++;
        }
    }

    i = lastNeutralized + 1;

    while ( i < baseFieldIdsSize ) {
        DexFieldId *pBaseFieldId = dexGetFieldId( pBaseDex, i );
        neutralizeFieldId( pBaseFieldId );
        i++;
    }
}

void neutralizeClassInterfaces( const DexFile *pBseDex,
        const DexClassDef *pClassDef ) {
    DexTypeList *pTypeList = dexGetInterfacesList( pBseDex, pClassDef );
    if ( pTypeList ) {
        for ( uint32_t j = 0; j < pTypeList->size; ++j ) {
            uint16_t typeId = pTypeList->list[j].typeIdx;
            uint32_t newInterfaceIdx;
            if ( hashmap_get( typeIdsMap, typeId,
                    &newInterfaceIdx ) == MAP_OK ) {
                pTypeList->list[j].typeIdx = (uint16_t) newInterfaceIdx;
            }
        }
    }
}

static void neutralizeAnnotationsSet( DexFile *pDex, uint32_t offset ) {
    DexAnnotationSetItem *pSet = dexGetAnnotationSetItem( pDex, offset );
    if ( !pSet ) {
        return;
    }

    for ( uint32_t i = 0; i < pSet->size; ++i ) {
        uint32_t newIdx;
        if ( hashmap_get( typeIdsMap, pSet->entries[i], &newIdx ) == MAP_OK ) {
            pSet->entries[i] = newIdx;
        }
    }
}

void neutralizeClassDefs( DexFile *pBaseDex ) {
    uint32_t size = pBaseDex->pHeader->classDefsSize;

    for ( uint32_t i = 0; i < size; ++i ) {
        DexClassDef *pClassDef = dexGetClassDef( pBaseDex, i );
        uint32_t newIdx;

        // classIdx
        if ( hashmap_get( typeIdsMap, pClassDef->classIdx, &newIdx ) ==
             MAP_OK ) {
            pClassDef->classIdx = newIdx;
        }

        //super class Idx
        if ( hashmap_get( typeIdsMap, pClassDef->superclassIdx, &newIdx ) ==
             MAP_OK ) {
            pClassDef->superclassIdx = newIdx;
        }

        // source_file_idx
        if ( hashmap_get( stringIdsMap, pClassDef->sourceFileIdx, &newIdx ) ==
             MAP_OK ) {
            pClassDef->sourceFileIdx = newIdx;
        }

        neutralizeClassInterfaces( pBaseDex, pClassDef );

        // annotations_off
        DexAnnotationsDirectoryItem *pDirectory = dexGetAnnotationsDirectoryItem(
                pBaseDex, pClassDef );
        if ( pDirectory != NULL ) {
            // annotations_off::class_annotation_off
            neutralizeAnnotationsSet( pBaseDex,
                    pDirectory->classAnnotationsOff );

            // annotations_off::field_annotations
            DexFieldAnnotationsItem *pFieldItems = dexGetFieldAnnotations(
                    pBaseDex,
                    pDirectory );
            if ( pFieldItems != NULL ) {
                for ( uint32_t j = 0; j < pDirectory->fieldsSize; ++j ) {
                    if ( hashmap_get( typeIdsMap, pFieldItems->fieldIdx,
                            &newIdx ) == MAP_OK ) {
                        pFieldItems->fieldIdx = newIdx;
                    }
                    neutralizeAnnotationsSet( pBaseDex,
                            pFieldItems->annotationsOff );
                }
            }

            // annotations_off::method_annotations
            DexMethodAnnotationsItem *pMethodItems = dexGetMethodAnnotations(
                    pBaseDex,
                    pDirectory );
            if ( pMethodItems != NULL ) {
                for ( uint32_t j = 0; j < pDirectory->methodsSize; ++j ) {
                    if ( hashmap_get( methodIdsMap, pMethodItems->methodIdx,
                            &newIdx ) == MAP_OK ) {
                        pMethodItems->methodIdx = newIdx;
                    }
                    neutralizeAnnotationsSet( pBaseDex,
                            pMethodItems->annotationsOff );
                }
            }

            // annotations_off::parameters_annotations
            DexParameterAnnotationsItem *pParamItems = dexGetParameterAnnotations(
                    pBaseDex, pDirectory );
            if ( pParamItems != NULL ) {
                for ( uint32_t j = 0; j < pDirectory->parametersSize; ++j ) {
                    if ( hashmap_get( methodIdsMap, pParamItems->methodIdx,
                            &newIdx )
                         == MAP_OK ) {
                        pParamItems->methodIdx = newIdx;
                    }
                    DexAnnotationSetRefList *pRefList =
                            dexGetParameterAnnotationSetRefList( pBaseDex,
                                    pParamItems );
                    if ( pRefList == NULL ) {
                        continue;
                    }
                    for ( uint32_t k = 0; k < pRefList->size; ++k ) {
                        neutralizeAnnotationsSet( pBaseDex,
                                pRefList->list[k].annotationsOff );
                    }
                }
            }
        }

        // class_data_off
        uint8_t *pData = dexGetClassData( pBaseDex, pClassDef );
        if ( pData != NULL ) {
            DexClassData *pClassData = dexReadAndVerifyClassData( &pData,
                    NULL );
            if ( pClassData == NULL ) {
                fprintf( stderr, "Failed to read class data\n" );
                return;
            }
            // class_data_off::instance_fields
            DexField *pInstanceFields = pClassData->instanceFields;
            if ( pInstanceFields != NULL ) {
                for ( uint32_t j = 0;
                      j < pClassData->header.instanceFieldsSize; ++j ) {
                    if ( hashmap_get( fieldIdsMap, pInstanceFields->fieldIdx,
                            &newIdx ) == MAP_OK ) {
                        pInstanceFields->fieldIdx = newIdx;
                    }
                }
            }

            // class_data_off::static_fields
            DexField *pStaticFields = pClassData->staticFields;
            if ( pStaticFields != NULL ) {
                for ( uint32_t j = 0;
                      j < pClassData->header.staticFieldsSize; ++j ) {
                    if ( hashmap_get( fieldIdsMap, pStaticFields->fieldIdx,
                            &newIdx ) == MAP_OK ) {
                        pStaticFields->fieldIdx = newIdx;
                    }
                }
            }

            // class_data_off::direct_methods
            DexMethod *pDirectMethods = pClassData->directMethods;
            if ( pDirectMethods != NULL ) {
                for ( uint32_t j = 0;
                      j < pClassData->header.directMethodsSize; ++j ) {
                    if ( hashmap_get( methodIdsMap, pDirectMethods->methodIdx,
                            &newIdx ) == MAP_OK ) {
                        pDirectMethods->methodIdx = newIdx;
                    }
                }
            }

            // class_data_off::virtual_methods
            DexMethod *pVirturalMethods = pClassData->virtualMethods;
            if ( pVirturalMethods != NULL ) {
                for ( uint32_t j = 0;
                      j < pClassData->header.directMethodsSize; ++j ) {
                    if ( hashmap_get( methodIdsMap, pVirturalMethods->methodIdx,
                            &newIdx ) == MAP_OK ) {
                        pVirturalMethods->methodIdx = newIdx;
                    }
                }
            }

            free( pClassData );
        }
    }
}


void neutralizeBase( DexFile *pBaseDex, const DexFile *pVariantDex ) {

    mapStringIds( pBaseDex, pVariantDex );

    mapTypeIds( pBaseDex, pVariantDex );

    mapProtoIds( pBaseDex, pVariantDex );

    mapFieldIds( pBaseDex, pVariantDex );

    neutralizeMethodIds( pBaseDex, pVariantDex );

    neutralizeClassDefs( pBaseDex );

    cleanup();
}