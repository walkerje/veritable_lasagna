#ifndef VL_MSGPACK_IO_H
#define VL_MSGPACK_IO_H

#include "vl_buffer.h"
#include "vl_stack.h"
#include "vl_msgpack.h"

typedef enum vl_msgpack_io_error_{
    VL_MSGPACK_IO_ERR_NONE,                 //No error. All good!
    VL_MSGPACK_IO_ERR_STACK_UNDERFLOW,      //Erroneous call to a container end encoding function.
    VL_MSGPACK_IO_ERR_UNEXPECTED_TOKEN,     //Expected a different token as "next" when decoding.
    VL_MSGPACK_IO_ERR_UNBALANCED_MAP,       //Map does not have an even number of values when encoding (must be key/value pairs)
    VL_MSGPACK_IO_ERR_UNEXPECTED_EOF,       //Found end of buffer at an unexpected point when decoding
} vl_msgpack_io_error;

/**
 * \brief MessagePack Format Encoder State
 *
 * This structure holds state for the encoding of information in the MessagePack format.
 * The encoder is completely compliant with the MessagePack spec.
 *
 * All functions used to encode have signatures among vlMsgPackIOEncode*.
 * The encoding API is loosely modeled on OpenGL 1.x immediate mode.
 *
 * This encoder attempts to generates the smallest possible (smallest size) result in all cases.
 */
typedef struct vl_msgpack_encoder_{
    //Holds state of current structure (map/array), including how many elements.
    vl_stack                        stateStack;
    //Holds encoded data.
    vl_buffer                       buffer;

    vl_msgpack_io_error             error;
    vl_msgpack_type                 errorType;
    vl_dsidx_t                      errorDepth;
} vl_msgpack_encoder;

/**
 * \brief Initializes the specified MessagePack encoder.
 *
 * This function initializes the given MessagePack encoder, preparing it for further encoding operations.
 * It must be called before encoding data into MessagePack format.
 *
 * \param enc Pointer to the MessagePack encoder to initialize.
 */
void vlMsgPackIOEncoderInit(vl_msgpack_encoder* enc);

/**
 * \brief Frees the specified MessagePack encoder.
 *
 * This function frees the resources allocated for the given MessagePack encoder. It must be called after
 * the encoder has been initialized with \ref vlMsgPackIOEncoderInit. This function does not deallocate the encoder structure.
 *
 * \param enc Pointer to the MessagePack encoder to free.
 */
void vlMsgPackIOEncoderFree(vl_msgpack_encoder* enc);

/**
 * \brief Allocates and initializes a MessagePack encoder.
 *
 * This function allocates memory for a new MessagePack encoder and initializes it. The returned pointer
 * must be freed using \ref vlMsgPackIOEncoderDelete.
 *
 * \sa vlMsgPackIOEncoderDelete
 * \return Pointer to the allocated and initialized MessagePack encoder.
 */
vl_msgpack_encoder* vlMsgPackIOEncoderNew();

/**
 * \brief Deletes the specified MessagePack encoder.
 *
 * This function deletes the given MessagePack encoder, freeing all associated resources. It must be called
 * after the encoder has been created with \ref vlMsgPackIOEncoderNew. The function deallocates the entire encoder structure.
 *
 * \sa vlMsgPackIOEncoderNew
 * \param enc Pointer to the MessagePack encoder to delete.
 */
void vlMsgPackIOEncoderDelete(vl_msgpack_encoder* enc);

/**
 * \brief Clears the state of the MessagePack encoder.
 *
 * This function resets the encoder to its initial state.
 *
 * \param enc Pointer to the encoder context.
 */
void vlMsgPackIOEncoderClear(vl_msgpack_encoder* enc);

/**
 * \brief Begins encoding a map in the MessagePack stream.
 *
 * This function serializes the beginning of a map (key-value pair container) into the MessagePack format.
 * It should be followed by calls to encode individual keys and values before the map is closed with \ref vlMsgPackIOEncodeMapEnd.
 *
 * \param enc Pointer to the encoder context.
 */
void vlMsgPackIOEncodeMapBegin(vl_msgpack_encoder* enc);

/**
 * \brief Ends encoding a map in the MessagePack stream.
 *
 * This function serializes the end of a map (key-value pair container) into the MessagePack format.
 * It follows \ref vlMsgPackIOEncodeMapBegin and should be called once all key-value pairs for the map have been encoded.
 *
 * This resolves the smallest possible encoding for the map.
 *
 * \param enc Pointer to the encoder context.
 */
void vlMsgPackIOEncodeMapEnd(vl_msgpack_encoder* enc);

/**
 * \brief Begins encoding an array in the MessagePack stream.
 *
 * This function serializes the beginning of an array (list of values) into the MessagePack format.
 * It should be followed by calls to encode individual values before the array is closed with \ref vlMsgPackIOEncodeArrayEnd.
 *
 * \param enc Pointer to the encoder context.
 */
void vlMsgPackIOEncodeArrayBegin(vl_msgpack_encoder* enc);

/**
 * \brief Ends encoding an array in the MessagePack stream.
 *
 * This function serializes the end of an array (list of values) into the MessagePack format.
 * It follows \ref vlMsgPackIOEncodeArrayBegin and should be called once all elements of the array have been encoded.
 *
 * This resolves the smallest possible encoding for the array.
 *
 * \param enc Pointer to the encoder context.
 */
void vlMsgPackIOEncodeArrayEnd(vl_msgpack_encoder* enc);

/**
 * \brief Encodes a boolean value into the MessagePack stream.
 *
 * This function serializes a boolean value (true or false) into the MessagePack format and appends it to
 * the encoder's buffer.
 *
 * \param enc Pointer to the encoder context.
 * \param value The boolean value to encode.
 */
void vlMsgPackIOEncodeBool(vl_msgpack_encoder* enc, vl_bool_t value);

/**
 * \brief Encodes a string's length into the MessagePack stream.
 *
 * This function serializes the length of a string value into the MessagePack format. It is often used in
 * combination with the string value itself for encoding large strings efficiently.
 *
 * \param enc Pointer to the encoder context.
 * \param value Pointer to the string to encode.
 * \param len The length of the string to encode.
 */
void vlMsgPackIOEncodeStringLen(vl_msgpack_encoder* enc, const char* value, vl_uint32_t len);

/**
 * \brief Encodes a UTF-8 string value into the MessagePack stream.
 *
 * This function serializes a string value into the MessagePack format and appends it to the encoder's buffer.
 *
 * \param enc Pointer to the encoder context.
 * \param value The string value to encode.
 */
static inline void      vlMsgPackIOEncodeString     (vl_msgpack_encoder* enc, const char* value){
    vlMsgPackIOEncodeStringLen(enc, value, strlen(value));
}

/**
 * \brief Encodes a 32-bit floating point value into the MessagePack stream.
 * 
 * This function serializes a 32-bit floating point value into the MessagePack format and appends it to
 * the encoder's buffer.
 * 
 * \param enc Pointer to the encoder context.
 * \param value The 32-bit floating point value to encode.
 */
void vlMsgPackIOEncodeFloat32(vl_msgpack_encoder* enc, vl_float32_t value);

/**
 * \brief Encodes a 64-bit floating point value into the MessagePack stream.
 * 
 * This function serializes a 64-bit floating point value into the MessagePack format and appends it to
 * the encoder's buffer.
 * 
 * \param enc Pointer to the encoder context.
 * \param value The 64-bit floating point value to encode.
 */
void vlMsgPackIOEncodeFloat64(vl_msgpack_encoder* enc, vl_float64_t value);

/**
 * \brief Encodes a binary value into the MessagePack stream.
 * 
 * This function serializes a binary data value into the MessagePack format and appends it to the encoder's buffer.
 * The length of the binary data must be provided.
 * 
 * \param enc Pointer to the encoder context.
 * \param value Pointer to the binary data to encode.
 * \param len The length of the binary data to encode.
 */
void vlMsgPackIOEncodeBinary(vl_msgpack_encoder* enc, const void* value, vl_uint32_t len);

/**
 * \brief Encodes an extension type value into the MessagePack stream.
 * 
 * This function serializes an extension type (custom format) into the MessagePack format and appends it
 * to the encoder's buffer. The extension type identifier and the associated value are serialized.
 * 
 * \param enc Pointer to the encoder context.
 * \param type The extension type identifier.
 * \param value Pointer to the extension data to encode.
 * \param len The length of the extension data.
 */
void vlMsgPackIOEncodeExt(vl_msgpack_encoder* enc, vl_int8_t type, const void* value, vl_uint32_t len);

/**
 * \brief Encodes a signed integer value into the MessagePack stream.
 * 
 * This function serializes a signed integer value into the MessagePack format and appends it to the encoder's buffer.
 * 
 * \param enc Pointer to the encoder context.
 * \param value The signed integer value to encode.
 */
void vlMsgPackIOEncodeInt(vl_msgpack_encoder* enc, vl_ilarge_t value);

/**
 * \brief Encodes an unsigned integer value into the MessagePack stream.
 * 
 * This function serializes an unsigned integer value into the MessagePack format and appends it to the encoder's buffer.
 * 
 * \param enc Pointer to the encoder context.
 * \param value The unsigned integer value to encode.
 */
void vlMsgPackIOEncodeUInt(vl_msgpack_encoder* enc, vl_ularge_t value);

/**
 * \brief Encodes a nil value into the MessagePack stream.
 * 
 * This function serializes a `nil` value (similar to `null`) into the MessagePack format and appends it
 * to the encoder's buffer.
 * 
 * \param enc Pointer to the encoder context.
 */
void vlMsgPackIOEncodeNil(vl_msgpack_encoder* enc);

/**
 * \brief Encodes from a MessagePack DOM.
 * \param enc Pointer to the encoder context.
 * \param src Pointer to the source MessagePack DOM.
 * \param curIter iterator to fully encode.
 */
void                    vlMsgPackIOEncodeFromDOM(vl_msgpack_encoder* encoder, vl_msgpack* src, vl_msgpack_iter curIter);

/**
 * \brief A MessagePack Token.
 *
 * Represents a single decoded value from a stream of encoded MessagePack values.
 */
typedef struct vl_msgpack_io_token_{
    /**
     * \private
     */
    union{
        /**
         * \private
         */
        struct{
            vl_dsidx_t      elements;
        } array;

        /**
         * \private
         */
        struct{
            vl_dsidx_t      keyValuePairs;
        } map;

        /**
         * \private
         */
        struct{
            vl_ilarge_t     value;
        } integer;

        /**
         * \private
         */
        struct{
            vl_ularge_t     value;
        } uinteger;

        /**
         * \private
         */
        struct{
            vl_float32_t    value;
        } float32;

        /**
         * \private
         */
        struct{
            vl_float64_t    value;
        } float64;

        /**
         * \private
         */
        struct{
            vl_bool_t       value;
        } boolean;

        /**
         * \private
         */
        struct{
            //UTF-8 Encoded
            const char*     ptr;
            vl_uint32_t     length;
        } string;

        /**
         * \private
         */
        struct{
            const void*     ptr;
            vl_uint32_t     length;
        } binary;

        /**
         * \private
         */
        struct{
            vl_int8_t       extType;
            const void*     ptr;
            vl_uint32_t     length;
        } ext;
    };

    vl_msgpack_type type;
} vl_msgpack_io_token;

/**
 * \brief A MessagePack decoder.
 *
 * The only job of the decoder is to produce a stream of decoded tokens from an arbitrary input buffer.
 */
typedef struct vl_msgpack_decoder_{
    const vl_uint8_t*       srcMem;
    vl_memsize_t            srcLen;
    vl_dsoffs_t             srcOffset;

    vl_msgpack_io_error     error;              //Error.
    vl_msgpack_type         errorType;          //Type associated with error.
    vl_dsoffs_t             errorOffset;        //Offset in source buffer where error occurred.
} vl_msgpack_decoder;

/**
 * \brief Starts the MessagePack decoding process.
 *
 * This function initializes the MessagePack decoder with the given source memory and length. It prepares
 * the decoder for further operations, such as token extraction or decoding to a DOM.
 *
 * \param dec Pointer to the MessagePack decoder to initialize.
 * \param srcMem Pointer to the source memory containing the MessagePack data to decode.
 * \param srcLen Length of the source memory to decode.
 */
void vlMsgPackIODecoderStart(vl_msgpack_decoder* dec, const void* srcMem, vl_memsize_t srcLen);

/**
 * \brief Extracts the next token from the MessagePack stream.
 *
 * This function decodes the next token from the MessagePack data. It returns \c true if a token was successfully
 * extracted, or \c false if there are no more tokens or the decoding process encounters an error.
 *
 * \param decoder Pointer to the MessagePack decoder.
 * \param tkPtr Pointer to the token structure where the extracted token will be stored.
 * \return \c true if a token was successfully decoded, \c false otherwise.
 */
vl_bool_t vlMsgPackIODecoderNext(vl_msgpack_decoder* decoder, vl_msgpack_io_token* tkPtr);

/**
 * \brief Decodes MessagePack data into a DOM.
 *
 * This function decodes MessagePack data and inserts it into the specified DOM at the given parent node. The
 * decoded data is inserted using the provided key.
 *
 * \param decoder Pointer to the MessagePack decoder.
 * \param dest Pointer to the MessagePack DOM to populate.
 * \param parent The parent node in the DOM where the decoded data will be inserted.
 * \param key Pointer to the UTF-8 string key, or pointer to integer index.
 * \param keyLen Length of the key.
 * \return The iterator to the newly inserted element in the DOM.
 */
vl_msgpack_iter vlMsgPackIODecodeToDOM(vl_msgpack_decoder* decoder, vl_msgpack* dest, vl_msgpack_iter parent, const void* key, vl_memsize_t keyLen);

#endif //VL_MSGPACK_IO_H