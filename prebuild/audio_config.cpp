// =====================================================================================
//
//       Filename:  audio_config.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-14 9:32:40 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../lofi/core/include/l_base.hpp"
#include "../lofi/core/include/l_container.hpp"

static constexpr u32 BIT_DEPTH = 16;
static constexpr u32 SAMPLE_RATE = 48000;

template<typename T, size_t Size>
using Array = lofi::mem::ArrayContainerPolicy<T, Size, 8, lofi::mem::SubAllocPolicy>;

using frame_t = u16;

struct Sample {
  lofi::String name{};
  frame_t* frames = nullptr;
  size_t num_frames = 0;
  b8 delete_sample() {
    if(frames != nullptr) {
      free(frames);
      return true;
    }
    return false;
  }
};

struct WAVFileFormat {
  Array<Sample, KB(1)> samples{nullptr};
  u64 data_size = 0;

  b8 delete_samples() {
    const size_t num_samples = samples.get_size();
    for(size_t i = 0; i < num_samples; i++) {
      if(!samples[i].delete_sample()) {
        return false;
      }
    }
    return true;
  }
};

b8 write_main_header_file(WAVFileFormat * const format, const char* file_path) {
  FILE* file = nullptr;
  fopen_s(&file, file_path, "wb");
  if(file == nullptr) {
    PRINT_LINE("failed to open write file");
    return false;
  }

  fprintf(file, "#pragma once\n");
  fprintf(file, "#include \"rx_vocab.h\"\n");
  fprintf(file, "\nnamespace roxi {");
  fprintf(file, "\n\tnamespace audio {");
  fprintf(file, "\n\t\tnamespace samples {\n");
  fprintf(file, "\n\t\t\tinline constexpr u64 data_size = %llu;\n", format->data_size);

  const u32 num_samples = format->samples.get_size();
  fprintf(file, "\n\t\t\tinline constexpr u64 num_samples = %u;\n", num_samples);
  u64 sample_start_index = 0;
  fprintf(file, "\n\t\t\tinline constexpr u64 sample_handles[] {\n\t\t\t\t");
  for(size_t i = 0; i < num_samples; i++) {
    if(i != 0) [[likely]] {
      fprintf(file, ", ");
      if(!(i & 7)) {
        fprintf(file, "\n\t\t\t\t");
      }
    }
    fprintf(file, "%llu", sample_start_index);
    sample_start_index += format->samples[i].num_frames;
  }
  fprintf(file, "\n\t\t\t};\n\n\t\t\t");
  fprintf(file, "inline constexpr u64 sample_sizes[] {\n\t\t\t\t");
  for(size_t i = 0; i < num_samples; i++) {
    if(i != 0) [[likely]] {
      fprintf(file, ", ");
      if(!(i & 7)) {
        fprintf(file, "\n\t\t\t\t");
      }
    }
    fprintf(file, "%llu", format->samples[i].num_frames);
  }
  fprintf(file, "\n\t\t\t};\n\n\t\t\t");
  fprintf(file, "inline const char* sample_names[] {\n\t\t\t\t");
  for(size_t i = 0; i < num_samples; i++) {
    if(i != 0) [[likely]] {
      fprintf(file, ", ");
      if(!(i & 7)) {
        fprintf(file, "\n\t\t\t\t");
      }
    }
    lofi::String name = format->samples[i].name;
    name.size -= 4;
    name.str[name.size] = '\0';
    size_t delim = 0;
    for(size_t i = 0; i < name.size; i++) {
      if(name.str[i] == '\\' || name.str[i] == '/') {
        delim = i;
      }
    }
    name.str = name.str + delim + 1;
    fprintf(file, "\"%s\"", name.str);
  }
  fprintf(file, "\n\t\t\t};\n\n\t\t\t");
  fprintf(file, "inline constexpr u32 samples[] {\n\t\t\t\t");
  size_t accum = 0;
  for(size_t i = 0; i < num_samples; i++) {
    const size_t num_frames = format->samples[i].num_frames;
    for(size_t j = 0; j < num_frames; j++) {
      if(accum != 0) [[likely]] {
        fprintf(file, ", ");
        if(!(accum & 7)) {
          fprintf(file, "\n\t\t\t\t");
        }
      }
      fprintf(file, "%u", format->samples[i].frames[j]);
      accum++;
    }
  }

  fprintf(file, "\n\t\t\t};\n\t\t}// -------- end of namespace samples ---------\n\t}// -------- end of namespace audio ---------\n}// -------- end of namespace roxi ---------\n");
  fclose(file);
  return true;
}

b8 parse_wav_file(Sample* sample, const char* file_path) {
  sample->name = lofi::str_cstring(file_path);
  FILE* file;
  fopen_s(&file, file_path, "rb");
  if(file == nullptr) {
    return false;
  }

  char magic[4];
  i32 file_size;
  i32 format_length;
  i16 format_type;
  i16 num_channels;
  i32 sample_rate;
  i32 bytes_per_second;
  i16 block_align;
  i16 bits_per_sample;
  i32 data_size;
  i32 chunk_size;

  fread(magic, 1, 4, file);
  if (magic[0] != 'R' || magic[1] != 'I' || magic[2] != 'F' || magic[3] != 'F') {
    PRINT_LINE("RIFF encoding missing");
    PRINT("%c%c%c%c instead\n", magic[0], magic[1], magic[2], magic[3]);
    return false;
  }
  fread(&file_size, 4, 1, file);
  
  fread(magic, 1, 4, file);
  if (magic[0] != 'W' || magic[1] != 'A' || magic[2] != 'V' || magic[3] != 'E') {
    PRINT_LINE("WAVE encoding missing");
    PRINT("%c%c%c%c instead\n", magic[0], magic[1], magic[2], magic[3]);
    return false;
  }

  fread(magic, 1, 4, file);
  if (magic[0] != 'f' || magic[1] != 'm' || magic[2] != 't' || magic[3] != ' ') {
    PRINT_LINE("fmt... encoding missing");
    PRINT("%c%c%c%c instead\n", magic[0], magic[1], magic[2], magic[3]);
    return false;
  }

 	fread(&format_length, 4, 1, file); 
  fread(&format_type, 2, 1, file);
 	if (format_type != 1) {
    PRINT("format type should be 3 for floating point data, or 1 for PCM, current type = %u\n", format_type);
    return false;
  }
   
 fread(&num_channels, 2, 1, file);
 if(num_channels != 1) {
   PRINT("trying to read a wav file that is not mono but expected a mono file, ie too many channels, num_channels = %u\n", num_channels);
    return false;
 }
 
 	fread(&sample_rate, 4, 1, file);
  if (sample_rate != SAMPLE_RATE) {
    PRINT("expected %u for sample rate and got %u instead\n", SAMPLE_RATE, sample_rate);
    return false;
  }
 	fread(&bytes_per_second, 4, 1, file);
 	fread(&block_align, 2, 1, file);
  fread(&bits_per_sample, 2, 1, file);
  if (bits_per_sample != BIT_DEPTH) {
    PRINT("expected %u for bit depth and got %u instead\n", BIT_DEPTH, bits_per_sample);
    return false;
  }

 	fread(magic, 1, 4, file);
  if (magic[0] != 'd' || magic[1] != 'a' || magic[2] != 't' || magic[3] != 'a') {
    PRINT_LINE("\'data\' string missing");
    return false;
  }
  		
 	fread(&data_size, 4, 1, file);
  sample->num_frames = data_size / (bits_per_sample / 8);
  sample->frames = (frame_t*)malloc(data_size);

  if(sample->frames == nullptr) {
    PRINT_LINE("failed to allocate samples");
    return false;
  }
  fread(sample->frames, 1, data_size, file);

  fclose(file);
  return true;
}


int main(int argc, char** argv) {

  WAVFileFormat format{};
  format.samples.move_ptr(malloc(sizeof(Sample) * argc - 1));

  for(size_t i = 1; i < argc - 1; i++) {
    Sample* sample = format.samples.push(1);
    if(!parse_wav_file(sample, argv[i])) {
      format.delete_samples();
      free(format.samples.get_buffer());
      return 1;
    }
    format.data_size += sample->num_frames * BIT_DEPTH / 8;
  }

  if(!write_main_header_file(&format, argv[argc - 1])) {
    PRINT_LINE("failed to write c++ header");
    format.delete_samples();
    free(format.samples.get_buffer());
    return 1;
  }

  format.delete_samples();
  free(format.samples.get_buffer());
  return 0;
}
