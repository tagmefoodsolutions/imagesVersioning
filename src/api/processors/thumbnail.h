#pragma once

#include "../io/source.h"
#include "base.h"

#include <weserv/config.h>

namespace weserv::api::processors {

class Thumbnail : ImageProcessor {
 public:
    Thumbnail(std::shared_ptr<parsers::Query> query, const Config &config)
        : ImageProcessor(std::move(query)), config_(config) {}

    /**
     * Use any shrink-on-load features available in the file import library.
     * @param image The source image.
     * @param source Source to read from.
     * @return An image that may have shrunk.
     */
    VImage shrink_on_load(const VImage &image, const io::Source &source) const;

    VImage process(const VImage &image) const override;

 private:
    /**
     * Global config.
     */
    const Config &config_;

    /**
     * Load a formatted image from a source for a specified image type.
     * @tparam ImageType Image type.
     * @param source Source to read from.
     * @param options Any options to pass on to the load operation.
     * @return A new `VImage`.
     */
    template <enums::ImageType ImageType>
    VImage new_from_source(const io::Source &source,
                           vips::VOption *options) const;

    /**
     * Calculate the shrink factor, taking into account auto-rotate, the fit
     * mode, and so on. The hshrink/vshrink are the amount to shrink the input
     * image axes by in order for the output axes (ie. after rotation) to match
     * the required thumbnail width/height and fit mode.
     * @param width Input width.
     * @param height Input height.
     * @return The (hshrink, vshrink) factor as pair.
     */
    std::pair<double, double> resolve_shrink(int width, int height) const;

    /**
     * Just the common part of the shrink: the bit by which both axes must be
     * shrunk.
     * @param width Input width.
     * @param height Input height.
     * @return shrink factor
     */
    double resolve_common_shrink(int width, int height) const;

    /**
     * Find the best jpeg preload shrink.
     * @param width Input width.
     * @param height Input height.
     * @return The jpeg shrink level.
     */
    int resolve_jpeg_shrink(int width, int height) const;

    /**
     * Find the pyramid level, if it's a pyr tiff.
     * We just look for two or more pages following roughly /2 shrinks.
     * @param image The source image.
     * @param source Source to read from.
     * @param width Input width.
     * @param height Input height.
     * @return The pyramid level.
     */
    int resolve_tiff_pyramid(const VImage &image, const io::Source &source,
                             int width, int height) const;

    /**
     * Find the best openslide level.
     * @param image The source image.
     * @return The pyramid level.
     */
    /*int resolve_open_slide_level(const VImage &image) const;*/

    /**
     * Append which page and the amount of pages we need to render for loaders
     * that support this.
     * @param options The source options.
     */
    void append_page_options(vips::VOption *options) const;
};

}  // namespace weserv::api::processors
