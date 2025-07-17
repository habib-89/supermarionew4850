for (int i = 0; i < tile_idx; i++)
    {
        if (tile_type[i] == 'o')
        {
            Sprite coin_test = tiles[i];
            if (iCheckCollision(&golem, &coin_test))
            {
                tile_type[i] = '_';
                // iSetSpritePosition(&tiles[i], -100, -100);
                score += 10;
            }
        }
    }
    if (!levelComplete && iCheckCollision(&golem, &flag))
    {
        levelComplete = true;
        printf("Level Complete!\n");
        gameState = LEVEL_SELECT;
    }