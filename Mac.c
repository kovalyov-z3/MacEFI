#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Protocol/SimpleNetwork.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;
  EFI_HANDLE *HandleBuffer = NULL;
  UINTN HandleCount;
  UINTN Index;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;
  CHAR16 *DevicePathStr = NULL;

  // Locate all handles that support the Simple Network Protocol
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleNetworkProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to locate handles: %r\n", Status);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiSimpleNetworkProtocolGuid, (VOID**)&Snp);
    if (EFI_ERROR(Status)) {
      Print(L"Failed to get SNP: %r\n", Status);
      continue;
    }

    // Get the device path
    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID**)&DevicePath);
    if (EFI_ERROR(Status)) {
      Print(L"Failed to get device path: %r\n", Status);
      continue;
    }

    // Check if the device path indicates an embedded device
    DevicePathStr = ConvertDevicePathToText(DevicePath, TRUE, TRUE);
    if (DevicePathStr != NULL) {
      if (StrStr(DevicePathStr, L"PciRoot") != NULL || StrStr(DevicePathStr, L"Acpi") != NULL) {
        Print(L"Found embedded network interface!\n");
        Print(L"MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
          Snp->Mode->CurrentAddress.Addr[0],
          Snp->Mode->CurrentAddress.Addr[1],
          Snp->Mode->CurrentAddress.Addr[2],
          Snp->Mode->CurrentAddress.Addr[3],
          Snp->Mode->CurrentAddress.Addr[4],
          Snp->Mode->CurrentAddress.Addr[5]
        );
        FreePool(DevicePathStr); // Free memory allocated for device path string
        FreePool(HandleBuffer);  // Free handle buffer memory
        return EFI_SUCCESS; // Exit after finding the first embedded network interface
      }
      FreePool(DevicePathStr); // Free memory allocated for device path string
    }
  }

  // Free memory allocated for handle buffer
  FreePool(HandleBuffer);

  Print(L"No embedded network interface found.\n");
  return EFI_NOT_FOUND; // Return an error code if no embedded network interface is found
}