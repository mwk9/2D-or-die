#include "entity_s.h"
#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "think_functions.h"

typedef struct
{
	Uint32 maxEntities;
	Entity * entityList;
	Uint64 increment;
}EntityManager;

//static variables prevents any other file from accessing the variable
//similar to private in this case
static EntityManager entityManager = { 0, NULL, 0 };

struct think_function think_table[] =
{
	{move, "move"},
	{ NULL, NULL }
};

/**
 * @brief Entity system clean-up function
 */
void entitySystemClose()
{
	int i;
	//entityDeleteAll();
	if (entityManager.entityList != NULL)
	{
		for (i = 0; i < entityManager.maxEntities; i++)
		{
			entityFree(&entityManager.entityList[i]);
		}
		free(entityManager.entityList);
	}
	//entityManager.entityList = NULL;
	//entityManager.maxEntities = 0;
	memset(&entityManager, 0, sizeof(EntityManager));
	slog("Entity system closed.");
}

void entitySystemInit(Uint32 max)
{
	if (max <= 0)
	{
		slog("Cannot initialize an entity manager for zero or negative entities!");
		return;
	}
	memset(&entityManager, 0, sizeof(EntityManager));

	entityManager.entityList = (Entity *)malloc(sizeof(Entity) * max);
	if (!entityManager.entityList)
	{
		slog("Could not allocate memory for the entity list");
		entitySystemClose();
		return;
	}
	memset(entityManager.entityList, 0, sizeof(Entity) * max);
	entityManager.maxEntities = max;

	slog("Entity system initialized");
	//on exit clean-up stuff goes here
	atexit(entitySystemClose);
}

Entity * entityNew()
{
	int i;

	for (i = 0; i < entityManager.maxEntities; i++)
	{
		if (entityManager.entityList[i].inUse == 0)
		{
			slog("Found a suitible spot for an entity at (%i)", i);
			memset(&entityManager.entityList[i], 0, sizeof(Entity));
			entityManager.entityList[i].id = entityManager.increment++;
			entityManager.entityList[i].inUse = 1;
			vector2d_set(entityManager.entityList[i].scale, 1, 1);
			//entityManager.entityList[i].actor.color = vector4d(1, 1, 1, 1);
			return &entityManager.entityList[i];
		}
	}

	slog("Error: Out of entity addresses!");
	return NULL;
}

void entityDelete(Entity * thingThatDies)
{
	if (!thingThatDies)
	{
		slog("Error: Cannot delete an entity that does not exist!");
		return;
	}
	/*if (thingThatDies->mySprite != NULL)
	{
		if (thingThatDies->mySprite->texture != NULL)
		{
			slog("destroying texture");
			SDL_DestroyTexture(thingThatDies->mySprite->texture);
		}
	}*/
	if (thingThatDies->inUse)
	{
		thingThatDies->inUse = 0;
	}
	//slog("Deleting a thing...");
	memset(thingThatDies, 0, sizeof(Entity));
	entityFree(thingThatDies);
	//slog("Deleted!");
}

void entityFree(Entity * e)
{
	//int i;
	if (!e)
	{
		slog("Error: Cannot free something that doesn't exist!");
		return;
	}

	//function pointer to a free functions
	//sound free
	//free actor
	//particless

	//slog("Freeing something from memory.");
	e->inUse = 0;
	//memset(e, 0, sizeof(Entity));
}

void entityDeleteAll()
{
	int i = 0;

	for (i = 0; i < entityManager.maxEntities; i++)
	{
		entityDelete(&entityManager.entityList[i]);
	}
}

void entityUpdate(Entity * self)
{
	if (!(self))
	{
		slog("Cannot update an entity that does not exist");
		return;
	}
	if (!(self)->inUse)
	{
		slog("Cannot update an entity that is not in use");
		return;
	}
	
	vector2d_add((self)->velocity, (self)->velocity, (self)->acceleration);

	if ((self)->boundingBox != NULL)
	{
		(self)->boundingBox->x = (self)->position.x;
		(self)->boundingBox->y = (self)->position.y;
	}

	if ((self)->update != NULL)
	{
		(self)->update((self));
	}
}

void entityUpdateAll()
{
	int i;
	for (i = 0; i < entityManager.maxEntities; i++)
	{
		if (entityManager.entityList[i].inUse == 1)
		{
			entityUpdate(&entityManager.entityList[i]);
		}
	}
}

void entityDraw(Entity * self)
{
	if (!self)
	{
		slog("Cannot draw an entity that does not exist");
		return;
	}
	if (!self->inUse)
	{
		slog("Cannot draw an entity that is not in use");
		return;
	}
	if (!self->mySprite)
	{
		slog("Cannot draw an entity with no sprite...%d", self->id);
		return;
	}
	gf2d_sprite_draw(
		self->mySprite, 
		self->position, 
		&self->scale, 
		&self->scaleCenter, 
		&self->rotation, 
		&self->flip, 
		NULL, 
		self->currentFrame
	);

	if (self->instrumentSprite)
	{
		gf2d_sprite_draw(
			self->instrumentSprite,
			self->position,
			&self->scale,
			NULL,
			NULL,
			NULL,
			NULL,
			0
		);
	}
}

void entityDrawAll()
{
	int i;
	for (i = 0; i < entityManager.maxEntities; i++)
	{
		if (entityManager.entityList[i].inUse)
		{
			entityDraw(&entityManager.entityList[i]);
		}
	}
}

void entityIncrementCurrentFrame(Entity * self)
{
	if (!self)
	{
		slog("Error: Cannot increment frame on a non-exisiting entity");
		return;
	}
	if (!self->inUse)
	{
		slog("Error: Cannot increment frame on an entity not in use");
		return;
	}
	if (!self->mySprite)
	{
		slog("Error: Cannot increment frame on an entity with no sprite");
		return;
	}

	self->currentFrame = self->currentFrame + 0.05f;
	//slog("%f", self->currentFrame);
	if (self->currentFrame < self->minFrame)
	{
		self->currentFrame = self->minFrame;
	}
	if (self->currentFrame > self->maxFrame)
	{
		self->currentFrame = self->minFrame;
	}
}

void entityIncrementCurrentFrameAll()
{
	int i;
	for (i = 0; i < entityManager.maxEntities; i++)
	{
		if (entityManager.entityList[i].inUse)
		{
			entityIncrementCurrentFrame(&entityManager.entityList[i]);
		}
	}
}

Entity * entityLoadFromFile(FILE * file, Entity * new_entity)
{
	char buffer[512];
	char holder[512];
	if (!file)
	{
		slog("Cannot open file");
		return NULL;
	}
	if (!new_entity)
	{
		slog("Error: new_entity was null");
		return NULL;
	}
	rewind(file);
	
	/* Note to self!
	   Order matters here. Please strcmp variables
	   in the order they appear in the entity_s
	   struct. Thanks Future Matt ~<3 Why don't
	   you text me back anymore */
	while (fscanf(file, "%s", buffer) != EOF)
	{
		if (strcmp(buffer, "name:") == 0)
		{
			fscanf(file, "%s", buffer);
			strncpy(new_entity->name, buffer, MAX_CHARS);
			//fscanf(file, "%s", buffer);
			slog("name is (%s)", new_entity->name);
			//slog("2 name is (%s)", buffer);
			continue;
		}
		if (strcmp(buffer, "fav_thing:") == 0)
		{
			fscanf(file, "%s", buffer);
			strncpy(new_entity->favoriteThing, buffer, MAX_CHARS);
			//fscanf(file, "%s", buffer);
			slog("favThing is (%s)", new_entity->favoriteThing);
			continue;
		}
		if (strcmp(buffer, "sprite:") == 0)
		{
			fscanf(file, "%s", buffer);
			new_entity->mySprite = gf2d_sprite_load_all(buffer, 32, 32, 2);
			continue;
		}
		if (strcmp(buffer, "Instrument:") == 0)
		{
			fscanf(file, "%s", holder);
			if (strcmp(holder, "flute") == 0)
				new_entity->myInstrument = Instrument_Flute;
			else if (strcmp(holder, "clarinet") == 0)
				new_entity->myInstrument = Instrument_Clarinet;
			else if (strcmp(holder, "altosax") == 0)
				new_entity->myInstrument = Instrument_Alto_Saxophone;
			else if (strcmp(holder, "tenorsax") == 0)
				new_entity->myInstrument = Instrument_Tenor_Saxophone;
			else if (strcmp(holder, "trumpet") == 0)
				new_entity->myInstrument = Instrument_Trumpet;
			else if (strcmp(holder, "baritone") == 0)
				new_entity->myInstrument = Instrument_Baritone;
			else if (strcmp(holder, "tuba") == 0)
				new_entity->myInstrument = Instrument_Tuba;
			else if (strcmp(holder, "snaredrum") == 0)
				new_entity->myInstrument = Instrument_Snare_Drum;
			else if (strcmp(holder, "bassdrum") == 0)
				new_entity->myInstrument = Instrument_Bass_Drum;
			else if (strcmp(holder, "cgflag") == 0)
				new_entity->myInstrument = Instrument_Colorguard_Flag;
			else if (strcmp(holder, "cgrifle") == 0)
				new_entity->myInstrument = Instrument_Colorguard_Rifle;
			else if (strcmp(holder, "pmarimba") == 0)
				new_entity->myInstrument = Instrument_Pit_Marimba;
			else if (strcmp(holder, "pxylophone") == 0)
				new_entity->myInstrument = Instrument_Pit_Xylophone;
			else if (strcmp(holder, "pguitar") == 0)
				new_entity->myInstrument = Instrument_Pit_Guitar;
			else if (strcmp(holder, "psynth") == 0)
				new_entity->myInstrument = Instrument_Pit_Synthesizer;
			else if (strcmp(holder, "pgong") == 0)
				new_entity->myInstrument = Instrument_Pit_Gong;
			else
				new_entity->myInstrument = Instrument_Unassigned;
			slog("instrument (%d)", new_entity->myInstrument);
			continue;
		}
		if (strcmp(buffer, "instrumentSprite:") == 0)
		{
			fscanf(file, "%s", buffer);
			strncpy(new_entity->instrumentSpriteFilePath, buffer, MAX_FILEPATH_CHARS);
			slog("instrument sprite file (%s)", new_entity->instrumentSpriteFilePath);
		}
		if (strcmp(buffer, "statMarching:") == 0)
		{
			fscanf(file, "%i", &new_entity->statMarching);
			//fscanf(file, "%s", buffer);
			slog("stat marching (%i)", new_entity->statMarching);
			continue;
		}
		if (strcmp(buffer, "statMusic:") == 0)
		{
			fscanf(file, "%i", &new_entity->statMusic);
			slog("stat music (%i)", new_entity->statMusic);
			continue;
		}
		if (strcmp(buffer, "statMorale:") == 0)
		{
			fscanf(file, "%i", &new_entity->statMorale);
			slog("stat morale (%i)", new_entity->statMorale);
			continue;
		}
		if (strcmp(buffer, "statMotivation:") == 0)
		{
			fscanf(file, "%i", &new_entity->statMotivation);
			slog("stat motivation (%i)", new_entity->statMotivation);
			continue;
		}
		if (strcmp(buffer, "sectionLeader:") == 0)
		{
			fscanf(file, "%i", &new_entity->isSectionLeader);
			slog("is section leader (%i)", new_entity->isSectionLeader);
			continue;
		}

		//fscanf(file, "%s", buffer);
		fgets(buffer, sizeof(buffer), file);
		//fscanf(file, "%s", holder);
		//slog("%s, %s", buffer, holder);
	}

	//fscanf(file, "name: %s", buffer);
	//slog("%s", buffer);
	//fscanf(file, "favThing: %s", buffer);
	//slog("%s", buffer);
	return new_entity;
}

void entityLoadAllFromFile(FILE * file, TileMap * map)
{
	Entity * currNew = NULL;
	char buffer[512];
	FILE * entityFile = NULL;
	int i = 0;
	if (!file)
	{
		slog("could not open file");
		return;
	}

	while (fscanf(file, "%s", buffer) != EOF)
	{
		entityFile = fopen(buffer, "r");
		if (!entityFile)
		{
			slog("bad filename (%s)", buffer);
			//fclose(entityFile);
		}
		else
		{
			currNew = entityNew();
			entityLoadFromFile(entityFile, currNew);
			fclose(entityFile);
			currNew->instrumentSprite = gf2d_sprite_load_all(&currNew->instrumentSpriteFilePath, 32, 32, 1);
			currNew->boundingBox = rect_new(currNew->position.x, currNew->position.y, 64, 64);
			currNew->scale = vector2d(2, 2);
			currNew->currentFrame = 0;
			currNew->minFrame = 0;
			currNew->maxFrame = 2;
			while (1)
			{
				i = random_int(0, map->width * map->height);
				if (map->space[i] != 0)
				{
					continue;
				}
				currNew->currentPosition = i;
				currNew->position = vector2d((i % 18) * 64, (i / 18) * 64);
				map->space[i] = 1;
				break;
			}

		}
	}
}

Entity * entityCheckCollisionInAll(int mx, int my)
{
	int i = 0;

	for (i = 0; i < entityManager.maxEntities; i++)
	{
		if (entityManager.entityList[i].inUse > 0)
		{
			if (point_in_rect(mx, my, entityManager.entityList[i].boundingBox))
			{
				return &entityManager.entityList[i];
			}
		}
	}

	return NULL;
}

char * entityGetInstrumentName(Entity * e)
{
	char buffer[512];
	if (!e)
	{
		slog("Error: could not get instrument from entity because entity was null");
		return NULL;
	}

	switch (e->myInstrument)
	{
	case Instrument_Flute:
		strncpy(buffer, "Flute", 512);
		break;
	case Instrument_Clarinet:
		strncpy(buffer, "Clarinet", 512);
		break;
	case Instrument_Alto_Saxophone:
		strncpy(buffer, "Alto Saxophone", 512);
		break;
	case Instrument_Tenor_Saxophone:
		strncpy(buffer, "Tenor Saxophone", 512);
		break;
	case Instrument_Trumpet:
		strncpy(buffer, "Trumpet", 512);
		break;
	case Instrument_Baritone:
		strncpy(buffer, "Baritone", 512);
		break;
	case Instrument_Tuba:
		strncpy(buffer, "Tuba", 512);
		break;
	case Instrument_Snare_Drum:
		strncpy(buffer, "Snare Drum", 512);
		break;
	case Instrument_Bass_Drum:
		strncpy(buffer, "Bass Drum", 512);
		break;
	case Instrument_Pit_Marimba:
		strncpy(buffer, "Marimba", 512);
		break;
	case Instrument_Pit_Xylophone:
		strncpy(buffer, "Xylophone", 512);
		break;
	case Instrument_Pit_Guitar:
		strncpy(buffer, "Guitar", 512);
		break;
	case Instrument_Pit_Synthesizer:
		strncpy(buffer, "Synthesizer", 512);
		break;
	case Instrument_Pit_Gong:
		strncpy(buffer, "Gong", 512);
		break;
	case Instrument_Colorguard_Flag:
		strncpy(buffer, "Flag Twirler", 512);
		break;
	case Instrument_Colorguard_Rifle:
		strncpy(buffer, "Rifle Tosser", 512);
		break;
	default:
		strncpy(buffer, "I have no idea, man...", 512);
		break;
	}

	return buffer;
}
