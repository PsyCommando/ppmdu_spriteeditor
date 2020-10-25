#ifndef TREEELEMTYPES_HPP
#define TREEELEMTYPES_HPP

/*
 * The possible kinds of data stored in our abstract data model
*/
enum struct [[maybe_unused]] eTreeElemDataType
{
    None,
    spritecontainer,
    sprite,
    palette,
    images,
    image,
    frames,
    frame,
    framepart,
    animSequences,
    animSequence,
    animTable,
    animGroup,
    animGroups,
    animFrame,
    animSequenceRef,
    effectOffsetSets,
    effectOffsetSet,
    effectOffset,

    INVALID,
};

#endif // TREEELEMTYPES_HPP
