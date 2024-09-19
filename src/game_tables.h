#ifndef game_tables_h_INCLUDED
#define game_tables_h_INCLUDED

#define STONE_KINDS(X, ...) \
	X(Granite, ## __VA_ARGS__) \
	X(Marble, ## __VA_ARGS__) \
	X(Limestone, ## __VA_ARGS__)

#define ANIMAL_BODY_PARTS(X, ...) \
  X(torso               , 6, 40, ## __VA_ARGS__) \
  X(neck                , 5, 25, ## __VA_ARGS__) \
  X(head                , 5, 25, ## __VA_ARGS__) \
  X(skull               , 5, 25, ## __VA_ARGS__) \
  X(brain               , 4, 10, ## __VA_ARGS__) \
  X(left_eye            , 4, 10, ## __VA_ARGS__) \
  X(right_eye           , 4, 10, ## __VA_ARGS__) \
  X(left_ear            , 4, 12, ## __VA_ARGS__) \
  X(right_ear           , 4, 12, ## __VA_ARGS__) \
  X(nose                , 4, 10, ## __VA_ARGS__) \
  X(jaw                 , 5, 20, ## __VA_ARGS__) \
  X(tongue              , 4, 10, ## __VA_ARGS__) \
  X(waist               , 4, 10, ## __VA_ARGS__) \
  X(spine               , 5, 25, ## __VA_ARGS__) \
  X(ribcage             , 5, 30, ## __VA_ARGS__) \
  X(sternum             , 5, 20, ## __VA_ARGS__) \
  X(stomach             , 5, 20, ## __VA_ARGS__) \
  X(heart               , 4, 15, ## __VA_ARGS__) \
  X(left_lung           , 4, 15, ## __VA_ARGS__) \
  X(right_lung          , 4, 15, ## __VA_ARGS__) \
  X(left_kidney         , 4, 15, ## __VA_ARGS__) \
  X(right_kidney        , 4, 15, ## __VA_ARGS__) \
  X(liver               , 5, 20, ## __VA_ARGS__) \
  X(left_shoulder       , 5, 30, ## __VA_ARGS__) \
  X(right_shoulder      , 5, 30, ## __VA_ARGS__) \
  X(left_clavicle       , 5, 25, ## __VA_ARGS__) \
  X(right_clavicle      , 5, 25, ## __VA_ARGS__) \
  X(left_arm            , 5, 30, ## __VA_ARGS__) \
  X(right_arm           , 5, 30, ## __VA_ARGS__) \
  X(left_humerus        , 5, 25, ## __VA_ARGS__) \
  X(right_humerus       , 5, 25, ## __VA_ARGS__) \
  X(left_radius         , 5, 20, ## __VA_ARGS__) \
  X(right_radius        , 5, 20, ## __VA_ARGS__) \
  X(left_hand           , 5, 20, ## __VA_ARGS__) \
  X(right_hand          , 5, 20, ## __VA_ARGS__) \
  X(left_pinky          , 3,  7, ## __VA_ARGS__) \
  X(right_pinky         , 3,  7, ## __VA_ARGS__) \
  X(left_ring_finger    , 3,  7, ## __VA_ARGS__) \
  X(right_ring_finger   , 3,  7, ## __VA_ARGS__) \
  X(left_middle_finger  , 3,  7, ## __VA_ARGS__) \
  X(right_middle_finger , 3,  7, ## __VA_ARGS__) \
  X(left_index_finger   , 3,  7, ## __VA_ARGS__) \
  X(right_index_finger  , 3,  7, ## __VA_ARGS__) \
  X(left_thumb          , 3,  7, ## __VA_ARGS__) \
  X(right_thumb         , 3,  7, ## __VA_ARGS__) \
  X(pelvis              , 5, 25, ## __VA_ARGS__) \
  X(left_leg            , 5, 30, ## __VA_ARGS__) \
  X(right_leg           , 5, 30, ## __VA_ARGS__) \
  X(left_femur          , 5, 25, ## __VA_ARGS__) \
  X(right_femur         , 5, 25, ## __VA_ARGS__) \
  X(left_tibia          , 5, 25, ## __VA_ARGS__) \
  X(right_tibia         , 5, 25, ## __VA_ARGS__) \
  X(left_foot           , 5, 25, ## __VA_ARGS__) \
  X(right_foot          , 5, 25, ## __VA_ARGS__) \
  X(left_little_toe     , 3,  7, ## __VA_ARGS__) \
  X(right_little_toe    , 3,  7, ## __VA_ARGS__) \
  X(left_fourth_toe     , 3,  7, ## __VA_ARGS__) \
  X(right_fourth_toe    , 3,  7, ## __VA_ARGS__) \
  X(left_middle_toe     , 3,  7, ## __VA_ARGS__) \
  X(right_middle_toe    , 3,  7, ## __VA_ARGS__) \
  X(left_second_toe     , 3,  7, ## __VA_ARGS__) \
  X(right_second_toe    , 3,  7, ## __VA_ARGS__) \
  X(left_big_toe        , 3,  7, ## __VA_ARGS__) \
  X(right_big_toe       , 3,  7, ## __VA_ARGS__)

#endif // game_tables_h_INCLUDED
