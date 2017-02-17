/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 2009 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <Uefi.h>


#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>


#include <Protocol/AtaPassThru.h>
#include <Protocol/PciIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/DevicePathFromText.h>

#define   MAX_PCI_BUSS              40
#define   MAX_PCI_DEVICES           32
#define   MAX_PCI_FUNCTIONS         8

#define PCI_LIB_ADDRESS(Bus,Device,Function,Offset)   \
  (((Offset) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))


EFI_STATUS
EFIAPI
PrintDevicePath(
    IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
    )
{
    EFI_STATUS                            Status;
    EFI_DEVICE_PATH_PROTOCOL             *DevPathNode;
    CHAR16                               *Textdevicepath =NULL;
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL     *Device2TextProtocol=NULL;
                            


    Status=gBS->LocateProtocol(&gEfiDevicePathToTextProtocolGuid,NULL,(VOID**)&Device2TextProtocol);
    ASSERT_EFI_ERROR (Status);

    DevPathNode=DevicePath;
    if(!IsDevicePathEnd(DevPathNode))
    {
	    Print(L"DevicePath: (%d,%d)",DevPathNode->Type,DevPathNode->SubType);
	    DevPathNode=NextDevicePathNode(DevPathNode);
    }
    while(!IsDevicePathEnd(DevPathNode))
    {
	    Print(L"/(%d,%d)",DevPathNode->Type,DevPathNode->SubType);
	    DevPathNode=NextDevicePathNode(DevPathNode);
    }
    Print(L"\n");
    


    if(!IsDevicePathEnd(DevPathNode))
    {
	    Textdevicepath=Device2TextProtocol->ConvertDevicePathToText(DevPathNode,TRUE,TRUE);
	    Print(L"%s",Textdevicepath);
	    Print(L"\n\n");
	    if(Textdevicepath) gBS->FreePool(Textdevicepath);
    }

    return EFI_SUCCESS;
    
}

VOID
EFIAPI
EnableSataMSI(
  IN UINTN		  PciBusNum,
  IN UINTN        PciDeviceNum,
  IN UINTN        PciFunctionNum
  )
{
    PciWrite8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0xDB),(PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x34))&0xFD));
}


VOID
EFIAPI
DisableSataMSI(
  IN UINTN		  PciBusNum,
  IN UINTN        PciDeviceNum,
  IN UINTN        PciFunctionNum
  )
{
    PciWrite8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0xDB),(PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x34))|0x02));
}

VOID
EFIAPI
EnableSataMSIX(
  IN UINTN		  PciBusNum,
  IN UINTN        PciDeviceNum,
  IN UINTN        PciFunctionNum
  )
{
    PciWrite8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x8E),(PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x8E))&0xBF));
}


VOID
EFIAPI
DisableSataMSIX(
  IN UINTN		  PciBusNum,
  IN UINTN        PciDeviceNum,
  IN UINTN        PciFunctionNum
  )
{
    PciWrite8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x8E),(PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x8E))|0x40));
}


INTN
EFIAPI
CheckPciCapability(
  IN UINTN		  PciBusNum,
  IN UINTN        PciDeviceNum,
  IN UINTN        PciFunctionNum,
  IN BOOLEAN      SubPrint
  )
{
    UINT8         Value01;
    UINT32        Value02;
    UINT8         Capability = 0;

    Value01 = PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x34)); 
    if(SubPrint)
    {
	    Print(L"    ");
    }
    
    Print(L"    Reg0x34 = 0x%x\n",Value01);
    Capability = Value01;
    
    while((Capability!=0x0) && (Capability!=0xFF))
    {
        Value02 = PciRead32(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,Capability));
        if(SubPrint)
	    {
		    Print(L"    ");
	    }
        Print(L"    Reg0x%x = 0x%02x, ",Capability,Value02&0xff);
        switch(Value02&0xff)
        {
            case 0x0:
                Print(L"Capability ID\n");
                break;
                
            case 0x01:
                Print(L"Power Management\n");
                break;
                
            case 0x02:
                Print(L"Accelerated Graphics Port\n");
                break;
                
            case 0x03:
                Print(L"Vital Product Data\n");
                break;
                
            case 0x04:
                Print(L"Slot Identification\n");
                break;
                
            case 0x05:
                Print(L"Message Signalled Interrupts\n");
                break;
                
            case 0x06:
                Print(L"CompactPCI HotSwap\n");
                break;
                
            case 0x07:
                Print(L"PCI-X\n");
                break;
                
            case 0x08:
                Print(L"HyperTransport\n");
                break;
                
            case 0x09:
                Print(L"Vendor Specific(Function Level Reset for ZX SATA Capability Possibly)\n");
                break;
                
            case 0x0A:
                Print(L"Debug port\n");
                break;
                
            case 0x0B:
                Print(L"CompactPCI Central Resource Control\n");
                break;
                
            case 0x0C:
                Print(L"PCI Standard Hot-Plug Controller\n");
                break;
                
            case 0x0D:
                Print(L"Bridge Subsystem Vendor/Device ID\n");
                break;
                
            case 0x0E:
                Print(L"AGP Target PCI-PCI bridge\n");
                break;
                
            case 0x10:
                Print(L"PCI Express\n");
                break;
                
            case 0x11:
                Print(L"MSI-X\n");
                break;
                
            case 0x12:
                Print(L"Index-data Pair for ZX SATA Capability\n");
                break;    
                
            case 0x13:
                Print(L"PCI Advanced Features(Function Level Reset for ZX SATA Capability Possibly)\n");
                break;
                
            default:
   	            Print(L"Unspecific Features\n"); 
   	            break; 
        }
     
	    Capability = (UINT8)(Value02>>8);
    }

    return EFI_SUCCESS;
}


INTN
EFIAPI
CheckSataCapability(
  IN UINTN		  PciBusNum,
  IN UINTN        PciDeviceNum,
  IN UINTN        PciFunctionNum
  )
{
    Print(L"\n    Enable MSIX and Enable MSIX\n");
    EnableSataMSIX(PciBusNum,PciDeviceNum,PciFunctionNum);
    EnableSataMSI(PciBusNum,PciDeviceNum,PciFunctionNum);
    CheckPciCapability(PciBusNum,PciDeviceNum,PciFunctionNum,TRUE);

    Print(L"\n    Disable MSIX and Enable MSI\n");
    DisableSataMSIX(PciBusNum,PciDeviceNum,PciFunctionNum);
    EnableSataMSI(PciBusNum,PciDeviceNum,PciFunctionNum);
    CheckPciCapability(PciBusNum,PciDeviceNum,PciFunctionNum,TRUE);

    Print(L"\n    Enable MSIX and Disable MSI\n");
    EnableSataMSIX(PciBusNum,PciDeviceNum,PciFunctionNum);
    DisableSataMSI(PciBusNum,PciDeviceNum,PciFunctionNum);
    CheckPciCapability(PciBusNum,PciDeviceNum,PciFunctionNum,TRUE);

    Print(L"\n     Disable MSIX and Disable MSI\n");
    DisableSataMSIX(PciBusNum,PciDeviceNum,PciFunctionNum);
    DisableSataMSI(PciBusNum,PciDeviceNum,PciFunctionNum);
    CheckPciCapability(PciBusNum,PciDeviceNum,PciFunctionNum,TRUE);
    Print(L"\n");

    return EFI_SUCCESS;
}


INTN
EFIAPI
CheckSataController()
{
	//EFI_STATUS    Status;
    UINTN         PciBusNum;
    UINTN         PciDeviceNum;
    UINTN         PciFunctionNum;
    UINT8         Value01;
    UINT8         Value02;
    UINT16        VendorID;
    UINT16        DeviceID;
    

    for (PciBusNum = 0; PciBusNum < MAX_PCI_BUSS; PciBusNum++) 
    {
        for (PciDeviceNum = 0; PciDeviceNum < MAX_PCI_DEVICES; PciDeviceNum++) 
        {
            for (PciFunctionNum = 0; PciFunctionNum < MAX_PCI_FUNCTIONS; PciFunctionNum++) 
            {
	                Value01 = PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0xA));
	                Value02 = PciRead8(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0xB)); 
	                if((Value01==0x01 || Value01==0x04 || Value01==0x06)&&(Value02==0x01))
	                {
					    VendorID = PciRead16(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x0));
					    DeviceID = PciRead16(PCI_LIB_ADDRESS(PciBusNum,PciDeviceNum,PciFunctionNum,0x2));
		                Print(L"Found SATA Controller at Bus%x/Dev%x/Func%x, VendorID/DeviceID = 0x%x/0x%x, ",PciBusNum,PciDeviceNum,PciFunctionNum,VendorID,DeviceID);
                        switch(Value01)
                        {
	                        case 0x01:
			                    Print(L"IDE Mode\n");
			                    break;
			                case 0x06:
			                    Print(L"AHCI Mode\n");
			                    break;
			                default:
			                    Print(L"RAID Mode\n");
			                    break;
                        }
		                CheckPciCapability(PciBusNum,PciDeviceNum,PciFunctionNum,FALSE);
		                if((VendorID==0x1106 || VendorID==0x1d17) && (DeviceID==0x9002 || DeviceID==0x9083)){
					         CheckSataCapability(PciBusNum,PciDeviceNum,PciFunctionNum);
					    }
	                }
            }
        }
    }

    return EFI_SUCCESS;
}


/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param[in] Argc     The number of items in Argv.
  @param[in] Argv     Array of pointers to strings.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain(
  IN UINTN				Argc,
  IN CHAR16			  **Argv
  )
{
	EFI_STATUS          Status;

    if(Argc==1)
    {
    	Status=CheckSataController();
    }else if(Argc==4)
    {
    	Status=CheckPciCapability(StrDecimalToUintn(Argv[1]),StrDecimalToUintn(Argv[2]),StrDecimalToUintn(Argv[3]),FALSE);
    }else
    {
      	Print(L"Usage: SataCapability BusNum DevNum FuncNum\n");
      	Status=EFI_UNSUPPORTED;
    }
  
    return Status;
}
