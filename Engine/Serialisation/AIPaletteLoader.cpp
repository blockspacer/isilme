#include "IsilmePCH.h"
#include "AIPaletteLoader.h"
#include "Palette/AIPalette.h"
#include "Core/FactoryManager.h"
#include "Core/MasterFile.h"

namespace serialisation
{


	AIPaletteLoader::AIPaletteLoader()
	{

	}

	AIPaletteLoader::~AIPaletteLoader()
	{

	}

	EntryPtr AIPaletteLoader::LoadEntry( std::string filename )
	{
		ContainerEntry<AIPalette>* entry = new ContainerEntry<AIPalette>(filename);
		entry->data = FactoryManager::GetSingleton()->GetAIPalette();

		entry->data->Load(filename);

		return EntryPtr(entry);
	}

}