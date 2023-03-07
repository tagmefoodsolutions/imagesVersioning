#include "source.h"

#include "../exceptions/unreadable.h"  // for UnreadableImageException
#include <fstream>                     // for ifstream

namespace weserv::api::io {

#ifdef WESERV_ENABLE_TRUE_STREAMING
/* Class implementation */

// We need C linkage for this.
extern "C" {
G_DEFINE_TYPE(WeservSource, weserv_source, VIPS_TYPE_SOURCE);
}

static gint64 weserv_source_read_wrapper(VipsSource *source, void *data,
                                         size_t length) {
    auto weserv_source = WESERV_SOURCE(source)->source;

    return weserv_source->read(data, length);
}

static gint64 weserv_source_seek_wrapper(VipsSource *source, gint64 offset,
                                         int whence) {
    auto weserv_source = WESERV_SOURCE(source)->source;

    return weserv_source->seek(offset, whence);
}

static void weserv_source_class_init(WeservSourceClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    VipsObjectClass *object_class = VIPS_OBJECT_CLASS(klass);
    VipsSourceClass *source_class = VIPS_SOURCE_CLASS(klass);

    gobject_class->set_property = vips_object_set_property;
    gobject_class->get_property = vips_object_get_property;

    object_class->nickname = "source";
    object_class->description = "weserv source";

    source_class->read = weserv_source_read_wrapper;
    source_class->seek = weserv_source_seek_wrapper;

    // clang-format off
    VIPS_ARG_POINTER(klass, "source", 3,
                     "Source pointer",
                     "Pointer to source",
                     VIPS_ARGUMENT_REQUIRED_INPUT,
                     G_STRUCT_OFFSET(WeservSource, source));
    // clang-format on
}

static void weserv_source_init(WeservSource *source) {}

/* private API */

Source Source::new_from_pointer(std::unique_ptr<io::SourceInterface> source) {
    WeservSource *weserv_source = WESERV_SOURCE(
        g_object_new(WESERV_TYPE_SOURCE, "source", source.get(), nullptr));

    if (vips_object_build(VIPS_OBJECT(weserv_source)) != 0) {
        VIPS_UNREF(weserv_source);
        throw vips::VError();
    }

    return Source(weserv_source);
}

Source Source::new_from_file(const std::string &filename) {
    VipsSource *source = vips_source_new_from_file(filename.c_str());

    if (source == nullptr) {
        throw vips::VError();
    }

    return Source(source);
}

Source Source::new_from_buffer(const std::string &buffer) {
    VipsSource *source =
        vips_source_new_from_memory(buffer.c_str(), buffer.size());

    if (source == nullptr) {
        throw vips::VError();
    }

    return Source(source);
}
#else
#define SOURCE_BUFFER_SIZE 4096  // = (size_t) ngx_pagesize;

Source Source::new_from_pointer(std::unique_ptr<io::SourceInterface> source) {
    char temp_buffer[SOURCE_BUFFER_SIZE];
    std::string buffer;
    int64_t bytes_read;

    do {
        bytes_read = source->read(temp_buffer, SOURCE_BUFFER_SIZE);

        if (bytes_read == -1) {
            throw exceptions::UnreadableImageException(
                "read error while buffering image");
        }

        buffer.append(temp_buffer, bytes_read);
    } while (bytes_read > 0);

    return Source(buffer);
}

Source Source::new_from_file(const std::string &filename) {
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();

    return Source(buffer.str());
}

Source Source::new_from_buffer(const std::string &buffer) {
    return Source(buffer);
}
#endif

}  // namespace weserv::api::io
